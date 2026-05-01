#include "compiler_project/lex/automata.h"

#include <stdio.h>
#include <string.h>

#include "compiler_project/common/error.h"
#include "compiler_project/lex/regex_parser.h"

typedef struct {
    int start;
    int end;
} Fragment;

typedef struct {
    const char *text;
    size_t pos;
    NFA *nfa;
    int failed;
    CompilerError error;
} RegexBuildContext;

static void init_accept(AutomataAccept *accept) {
    memset(accept, 0, sizeof(*accept));
    accept->priority = CP_MAX_LEX_RULES + 1;
}

static void init_nfa(NFA *nfa) {
    int i;
    memset(nfa, 0, sizeof(*nfa));
    nfa->start_state = -1;
    for (i = 0; i < CP_MAX_AUTOMATA_STATES; ++i) {
        init_accept(&nfa->accepts[i]);
    }
}

static void init_dfa(DFA *dfa) {
    int i;
    int ch;
    memset(dfa, 0, sizeof(*dfa));
    dfa->start_state = 0;
    for (i = 0; i < CP_MAX_AUTOMATA_STATES; ++i) {
        init_accept(&dfa->accepts[i]);
        for (ch = 0; ch < CP_AUTOMATA_CHAR_COUNT; ++ch) {
            dfa->transitions[i][ch] = -1;
        }
    }
}

static int new_state(RegexBuildContext *ctx) {
    int state;
    if (ctx->nfa->state_count >= CP_MAX_AUTOMATA_STATES) {
        cp_make_error(&ctx->error, 2005, "Too many NFA states", -1, -1, "automata");
        ctx->failed = 1;
        return -1;
    }
    state = ctx->nfa->state_count++;
    init_accept(&ctx->nfa->accepts[state]);
    return state;
}

static int add_transition(NFA *nfa, int from, int to, int min_char, int max_char, int epsilon) {
    AutomataTransition *transition;
    if (nfa->transition_count >= CP_MAX_AUTOMATA_TRANSITIONS) {
        return 0;
    }
    transition = &nfa->transitions[nfa->transition_count++];
    transition->from = from;
    transition->to = to;
    transition->min_char = min_char;
    transition->max_char = max_char;
    transition->epsilon = epsilon;
    return 1;
}

static int current_char(const RegexBuildContext *ctx) {
    return (unsigned char) ctx->text[ctx->pos];
}

static int at_end_or_alt_or_group(const RegexBuildContext *ctx) {
    int ch = current_char(ctx);
    return ch == '\0' || ch == '|' || ch == ')';
}

static void fail_regex(RegexBuildContext *ctx, int code, const char *message) {
    if (!ctx->failed) {
        cp_make_error(&ctx->error, code, message, -1, (int) ctx->pos + 1, "automata");
        ctx->failed = 1;
    }
}

static Fragment empty_fragment(RegexBuildContext *ctx) {
    Fragment fragment;
    fragment.start = new_state(ctx);
    fragment.end = new_state(ctx);
    if (!ctx->failed && !add_transition(ctx->nfa, fragment.start, fragment.end, 0, 0, 1)) {
        fail_regex(ctx, 2006, "Too many NFA transitions");
    }
    return fragment;
}

static Fragment char_fragment(RegexBuildContext *ctx, int min_char, int max_char) {
    Fragment fragment;
    fragment.start = new_state(ctx);
    fragment.end = new_state(ctx);
    if (!ctx->failed && !add_transition(ctx->nfa, fragment.start, fragment.end, min_char, max_char, 0)) {
        fail_regex(ctx, 2006, "Too many NFA transitions");
    }
    return fragment;
}

static Fragment parse_expr(RegexBuildContext *ctx);

static void add_escape_transitions(RegexBuildContext *ctx, int start, int end, int escaped) {
    if (escaped == 's') {
        add_transition(ctx->nfa, start, end, ' ', ' ', 0);
        add_transition(ctx->nfa, start, end, '\t', '\t', 0);
        add_transition(ctx->nfa, start, end, '\n', '\n', 0);
        add_transition(ctx->nfa, start, end, '\r', '\r', 0);
        return;
    }
    add_transition(ctx->nfa, start, end, escaped, escaped, 0);
}

static Fragment escaped_fragment(RegexBuildContext *ctx) {
    Fragment fragment;
    int escaped;
    ctx->pos++;
    escaped = current_char(ctx);
    if (escaped == '\0') {
        fail_regex(ctx, 2001, "Dangling escape in regex");
        return empty_fragment(ctx);
    }
    ctx->pos++;
    fragment.start = new_state(ctx);
    fragment.end = new_state(ctx);
    if (!ctx->failed) {
        add_escape_transitions(ctx, fragment.start, fragment.end, escaped);
    }
    return fragment;
}

static Fragment class_fragment(RegexBuildContext *ctx) {
    Fragment fragment;
    int saw_item = 0;
    fragment.start = new_state(ctx);
    fragment.end = new_state(ctx);
    ctx->pos++;
    while (current_char(ctx) != '\0' && current_char(ctx) != ']') {
        int first;
        int last;
        if (current_char(ctx) == '\\') {
            ctx->pos++;
            first = current_char(ctx);
            if (first == '\0') {
                fail_regex(ctx, 2001, "Dangling escape in character class");
                return fragment;
            }
            ctx->pos++;
        } else {
            first = current_char(ctx);
            ctx->pos++;
        }
        last = first;
        if (current_char(ctx) == '-' && ctx->text[ctx->pos + 1] != '\0' && ctx->text[ctx->pos + 1] != ']') {
            ctx->pos++;
            last = current_char(ctx);
            ctx->pos++;
            if (last < first) {
                fail_regex(ctx, 2007, "Invalid character class range");
                return fragment;
            }
        }
        add_transition(ctx->nfa, fragment.start, fragment.end, first, last, 0);
        saw_item = 1;
    }
    if (current_char(ctx) != ']') {
        fail_regex(ctx, 2003, "Unclosed character class");
        return fragment;
    }
    ctx->pos++;
    if (!saw_item) {
        fail_regex(ctx, 2008, "Empty character class");
    }
    return fragment;
}

static Fragment parse_atom(RegexBuildContext *ctx) {
    int ch = current_char(ctx);
    if (ch == '(') {
        Fragment fragment;
        ctx->pos++;
        fragment = parse_expr(ctx);
        if (current_char(ctx) != ')') {
            fail_regex(ctx, 2004, "Unclosed group");
            return fragment;
        }
        ctx->pos++;
        return fragment;
    }
    if (ch == '[') {
        return class_fragment(ctx);
    }
    if (ch == '\\') {
        return escaped_fragment(ctx);
    }
    if (ch == '\0' || ch == ')' || ch == '|' || ch == '*' || ch == '+' || ch == '?') {
        fail_regex(ctx, 2009, "Expected regex atom");
        return empty_fragment(ctx);
    }
    ctx->pos++;
    return char_fragment(ctx, ch, ch);
}

static Fragment concat_fragments(RegexBuildContext *ctx, Fragment left, Fragment right) {
    if (!add_transition(ctx->nfa, left.end, right.start, 0, 0, 1)) {
        fail_regex(ctx, 2006, "Too many NFA transitions");
    }
    left.end = right.end;
    return left;
}

static Fragment alternate_fragments(RegexBuildContext *ctx, Fragment left, Fragment right) {
    Fragment result;
    result.start = new_state(ctx);
    result.end = new_state(ctx);
    add_transition(ctx->nfa, result.start, left.start, 0, 0, 1);
    add_transition(ctx->nfa, result.start, right.start, 0, 0, 1);
    add_transition(ctx->nfa, left.end, result.end, 0, 0, 1);
    add_transition(ctx->nfa, right.end, result.end, 0, 0, 1);
    return result;
}

static Fragment apply_postfix(RegexBuildContext *ctx, Fragment fragment, int op) {
    Fragment result;
    result.start = new_state(ctx);
    result.end = new_state(ctx);
    if (op == '*') {
        add_transition(ctx->nfa, result.start, fragment.start, 0, 0, 1);
        add_transition(ctx->nfa, result.start, result.end, 0, 0, 1);
        add_transition(ctx->nfa, fragment.end, fragment.start, 0, 0, 1);
        add_transition(ctx->nfa, fragment.end, result.end, 0, 0, 1);
    } else if (op == '+') {
        add_transition(ctx->nfa, result.start, fragment.start, 0, 0, 1);
        add_transition(ctx->nfa, fragment.end, fragment.start, 0, 0, 1);
        add_transition(ctx->nfa, fragment.end, result.end, 0, 0, 1);
    } else {
        add_transition(ctx->nfa, result.start, fragment.start, 0, 0, 1);
        add_transition(ctx->nfa, result.start, result.end, 0, 0, 1);
        add_transition(ctx->nfa, fragment.end, result.end, 0, 0, 1);
    }
    return result;
}

static Fragment parse_factor(RegexBuildContext *ctx) {
    Fragment fragment = parse_atom(ctx);
    while (!ctx->failed && (current_char(ctx) == '*' || current_char(ctx) == '+' || current_char(ctx) == '?')) {
        int op = current_char(ctx);
        ctx->pos++;
        fragment = apply_postfix(ctx, fragment, op);
    }
    return fragment;
}

static Fragment parse_term(RegexBuildContext *ctx) {
    Fragment result;
    int has_factor = 0;
    result.start = -1;
    result.end = -1;
    while (!ctx->failed && !at_end_or_alt_or_group(ctx)) {
        Fragment next = parse_factor(ctx);
        if (!has_factor) {
            result = next;
            has_factor = 1;
        } else {
            result = concat_fragments(ctx, result, next);
        }
    }
    if (!has_factor) {
        return empty_fragment(ctx);
    }
    return result;
}

static Fragment parse_expr(RegexBuildContext *ctx) {
    Fragment result = parse_term(ctx);
    while (!ctx->failed && current_char(ctx) == '|') {
        Fragment right;
        ctx->pos++;
        right = parse_term(ctx);
        result = alternate_fragments(ctx, result, right);
    }
    return result;
}

static void mark_accept(NFA *nfa, int state, const char *token_kind, int skip, int priority) {
    nfa->accepts[state].accepting = 1;
    nfa->accepts[state].skip = skip;
    nfa->accepts[state].priority = priority;
    snprintf(nfa->accepts[state].token_kind, sizeof(nfa->accepts[state].token_kind), "%s", token_kind);
}

static NFAResult build_rule_nfa(const char *regex_text, const char *token_kind, int skip, int priority) {
    NFAResult result;
    RegexBuildContext ctx;
    Fragment fragment;
    RegexResult regex_result = cp_parse_regex(regex_text);
    RESULT_SUCCESS(&result.base);
    init_nfa(&result.data);
    if (!regex_result.base.ok) {
        RESULT_FAILURE(&result.base, regex_result.base.errors[0]);
        return result;
    }
    memset(&ctx, 0, sizeof(ctx));
    ctx.text = regex_result.data;
    ctx.nfa = &result.data;
    /* Thompson construction is driven directly by the recursive-descent regex parser. */
    fragment = parse_expr(&ctx);
    if (!ctx.failed && current_char(&ctx) != '\0') {
        fail_regex(&ctx, 2010, "Unexpected regex suffix");
    }
    if (ctx.failed) {
        RESULT_FAILURE(&result.base, ctx.error);
        return result;
    }
    result.data.start_state = fragment.start;
    mark_accept(&result.data, fragment.end, token_kind, skip, priority);
    return result;
}

NFAResult cp_build_nfa(const char *regex_ast, const char *token_kind) {
    return build_rule_nfa(regex_ast, token_kind, 0, 0);
}

static int copy_rule_into_combined(NFA *combined, const NFA *rule) {
    int offset = combined->state_count;
    int i;
    if (combined->state_count + rule->state_count >= CP_MAX_AUTOMATA_STATES) {
        return 0;
    }
    for (i = 0; i < rule->state_count; ++i) {
        combined->accepts[offset + i] = rule->accepts[i];
    }
    combined->state_count += rule->state_count;
    for (i = 0; i < rule->transition_count; ++i) {
        const AutomataTransition *transition = &rule->transitions[i];
        if (!add_transition(
            combined,
            transition->from + offset,
            transition->to + offset,
            transition->min_char,
            transition->max_char,
            transition->epsilon
        )) {
            return 0;
        }
    }
    return add_transition(combined, combined->start_state, rule->start_state + offset, 0, 0, 1);
}

static void epsilon_closure(const NFA *nfa, const unsigned char *input, unsigned char *output) {
    int changed = 1;
    int i;
    memcpy(output, input, CP_MAX_AUTOMATA_STATES);
    while (changed) {
        changed = 0;
        for (i = 0; i < nfa->transition_count; ++i) {
            const AutomataTransition *transition = &nfa->transitions[i];
            if (transition->epsilon && output[transition->from] && !output[transition->to]) {
                output[transition->to] = 1;
                changed = 1;
            }
        }
    }
}

static void move_on_char(const NFA *nfa, const unsigned char *input, int ch, unsigned char *output) {
    int i;
    memset(output, 0, CP_MAX_AUTOMATA_STATES);
    for (i = 0; i < nfa->transition_count; ++i) {
        const AutomataTransition *transition = &nfa->transitions[i];
        if (!transition->epsilon &&
            input[transition->from] &&
            ch >= transition->min_char &&
            ch <= transition->max_char) {
            output[transition->to] = 1;
        }
    }
}

static int same_set(const unsigned char *left, const unsigned char *right) {
    return memcmp(left, right, CP_MAX_AUTOMATA_STATES) == 0;
}

static int find_dfa_state(unsigned char sets[][CP_MAX_AUTOMATA_STATES], int count, const unsigned char *set) {
    int i;
    for (i = 0; i < count; ++i) {
        if (same_set(sets[i], set)) {
            return i;
        }
    }
    return -1;
}

static int set_empty(const unsigned char *set) {
    int i;
    for (i = 0; i < CP_MAX_AUTOMATA_STATES; ++i) {
        if (set[i]) {
            return 0;
        }
    }
    return 1;
}

static AutomataAccept choose_accept(const NFA *nfa, const unsigned char *set) {
    AutomataAccept best;
    int i;
    init_accept(&best);
    for (i = 0; i < nfa->state_count; ++i) {
        if (set[i] && nfa->accepts[i].accepting &&
            (!best.accepting || nfa->accepts[i].priority < best.priority)) {
            best = nfa->accepts[i];
        }
    }
    return best;
}

DFAResult cp_determinize(const NFA *nfa) {
    DFAResult result;
    unsigned char sets[CP_MAX_AUTOMATA_STATES][CP_MAX_AUTOMATA_STATES];
    int processed = 0;
    RESULT_SUCCESS(&result.base);
    init_dfa(&result.data);
    if (nfa == NULL || nfa->start_state < 0) {
        result.data.state_count = 0;
        return result;
    }
    memset(sets, 0, sizeof(sets));
    sets[0][nfa->start_state] = 1;
    epsilon_closure(nfa, sets[0], sets[0]);
    result.data.state_count = 1;
    result.data.accepts[0] = choose_accept(nfa, sets[0]);
    /* Subset construction: each DFA state is an epsilon-closed set of NFA states. */
    while (processed < result.data.state_count) {
        int ch;
        for (ch = 0; ch < CP_AUTOMATA_CHAR_COUNT; ++ch) {
            unsigned char moved[CP_MAX_AUTOMATA_STATES];
            unsigned char closed[CP_MAX_AUTOMATA_STATES];
            int target;
            move_on_char(nfa, sets[processed], ch, moved);
            if (set_empty(moved)) {
                continue;
            }
            epsilon_closure(nfa, moved, closed);
            target = find_dfa_state(sets, result.data.state_count, closed);
            if (target < 0) {
                if (result.data.state_count >= CP_MAX_AUTOMATA_STATES) {
                    CompilerError error;
                    cp_make_error(&error, 2011, "Too many DFA states", -1, -1, "automata");
                    RESULT_FAILURE(&result.base, error);
                    return result;
                }
                target = result.data.state_count++;
                memcpy(sets[target], closed, CP_MAX_AUTOMATA_STATES);
                result.data.accepts[target] = choose_accept(nfa, closed);
            }
            result.data.transitions[processed][ch] = target;
        }
        processed++;
    }
    return result;
}

static int accept_equivalent(const AutomataAccept *left, const AutomataAccept *right) {
    return left->accepting == right->accepting &&
        left->skip == right->skip &&
        left->priority == right->priority &&
        strcmp(left->token_kind, right->token_kind) == 0;
}

DFAResult cp_minimize_dfa(const DFA *dfa) {
    DFAResult result;
    int group[CP_MAX_AUTOMATA_STATES];
    int new_group[CP_MAX_AUTOMATA_STATES];
    int group_count = 0;
    int changed = 1;
    int i;
    RESULT_SUCCESS(&result.base);
    init_dfa(&result.data);
    if (dfa == NULL || dfa->state_count <= 0) {
        result.data.state_count = 0;
        return result;
    }
    for (i = 0; i < dfa->state_count; ++i) {
        int g;
        group[i] = -1;
        for (g = 0; g < group_count; ++g) {
            int representative;
            for (representative = 0; representative < i; ++representative) {
                if (group[representative] == g) {
                    break;
                }
            }
            if (representative < i && accept_equivalent(&dfa->accepts[i], &dfa->accepts[representative])) {
                group[i] = g;
                break;
            }
        }
        if (group[i] < 0) {
            group[i] = group_count++;
        }
    }
    while (changed) {
        changed = 0;
        group_count = 0;
        /* Refine equivalence classes until transitions and accept metadata agree. */
        for (i = 0; i < dfa->state_count; ++i) {
            int candidate;
            new_group[i] = -1;
            for (candidate = 0; candidate < i; ++candidate) {
                int ch;
                int equivalent = accept_equivalent(&dfa->accepts[i], &dfa->accepts[candidate]);
                if (new_group[candidate] < 0) {
                    continue;
                }
                for (ch = 0; equivalent && ch < CP_AUTOMATA_CHAR_COUNT; ++ch) {
                    int left = dfa->transitions[i][ch];
                    int right = dfa->transitions[candidate][ch];
                    int left_group = left < 0 ? -1 : group[left];
                    int right_group = right < 0 ? -1 : group[right];
                    if (left_group != right_group) {
                        equivalent = 0;
                    }
                }
                if (equivalent) {
                    new_group[i] = new_group[candidate];
                    break;
                }
            }
            if (new_group[i] < 0) {
                new_group[i] = group_count++;
            }
        }
        for (i = 0; i < dfa->state_count; ++i) {
            if (group[i] != new_group[i]) {
                changed = 1;
            }
            group[i] = new_group[i];
        }
    }
    result.data.state_count = group_count;
    result.data.start_state = group[dfa->start_state];
    for (i = 0; i < dfa->state_count; ++i) {
        int ch;
        int g = group[i];
        if (!result.data.accepts[g].accepting && dfa->accepts[i].accepting) {
            result.data.accepts[g] = dfa->accepts[i];
        }
        for (ch = 0; ch < CP_AUTOMATA_CHAR_COUNT; ++ch) {
            if (dfa->transitions[i][ch] >= 0) {
                result.data.transitions[g][ch] = group[dfa->transitions[i][ch]];
            }
        }
    }
    return result;
}

DFAResult cp_build_dfa_from_lex_spec(const LexSpec *spec) {
    NFA combined;
    DFAResult dfa;
    DFAResult min_dfa;
    int i;
    init_nfa(&combined);
    combined.start_state = 0;
    combined.state_count = 1;
    if (spec == NULL) {
        CompilerError error;
        RESULT_SUCCESS(&dfa.base);
        init_dfa(&dfa.data);
        cp_make_error(&error, 2012, "Missing lex spec", -1, -1, "automata");
        RESULT_FAILURE(&dfa.base, error);
        return dfa;
    }
    for (i = 0; i < spec->rule_count; ++i) {
        NFAResult rule = build_rule_nfa(
            spec->rules[i].pattern,
            spec->rules[i].token_type,
            spec->rules[i].skip,
            spec->rules[i].priority
        );
        if (!rule.base.ok) {
            RESULT_SUCCESS(&dfa.base);
            init_dfa(&dfa.data);
            RESULT_FAILURE(&dfa.base, rule.base.errors[0]);
            return dfa;
        }
        if (!copy_rule_into_combined(&combined, &rule.data)) {
            CompilerError error;
            RESULT_SUCCESS(&dfa.base);
            init_dfa(&dfa.data);
            cp_make_error(&error, 2013, "Combined NFA is too large", -1, -1, "automata");
            RESULT_FAILURE(&dfa.base, error);
            return dfa;
        }
    }
    dfa = cp_determinize(&combined);
    if (!dfa.base.ok) {
        return dfa;
    }
    min_dfa = cp_minimize_dfa(&dfa.data);
    return min_dfa;
}

void cp_dump_dfa_text(const DFA *dfa, char *buffer, int buffer_size) {
    int i;
    int used = 0;
    if (buffer == NULL || buffer_size <= 0) {
        return;
    }
    buffer[0] = '\0';
    if (dfa == NULL) {
        snprintf(buffer, (size_t) buffer_size, "DFA: null\n");
        return;
    }
    used += snprintf(buffer + used, (size_t) (buffer_size - used), "states: %d\n", dfa->state_count);
    for (i = 0; i < dfa->state_count && used < buffer_size; ++i) {
        int ch;
        if (dfa->accepts[i].accepting) {
            used += snprintf(buffer + used, (size_t) (buffer_size - used),
                "state %d accept %s skip=%d priority=%d\n",
                i,
                dfa->accepts[i].token_kind,
                dfa->accepts[i].skip,
                dfa->accepts[i].priority);
        }
        for (ch = 0; ch < CP_AUTOMATA_CHAR_COUNT && used < buffer_size; ++ch) {
            if (dfa->transitions[i][ch] >= 0) {
                used += snprintf(buffer + used, (size_t) (buffer_size - used),
                    "state %d -- %c(%d) --> %d\n",
                    i,
                    ch >= 32 && ch <= 126 ? ch : '.',
                    ch,
                    dfa->transitions[i][ch]);
            }
        }
    }
}
