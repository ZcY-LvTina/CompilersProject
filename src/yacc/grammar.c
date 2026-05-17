#include "compiler_project/yacc/grammar.h"

#include <stdio.h>
#include <string.h>

#include "compiler_project/common/error.h"

#define CP_EPSILON_SYMBOL "__EPSILON__"

static int set_contains(const GrammarSymbolSet *set, const char *symbol) {
    int i;
    if (set == NULL || symbol == NULL) {
        return 0;
    }
    for (i = 0; i < set->count; ++i) {
        if (strcmp(set->items[i], symbol) == 0) {
            return 1;
        }
    }
    return 0;
}

static int set_add(GrammarSymbolSet *set, const char *symbol) {
    if (set == NULL || symbol == NULL || set_contains(set, symbol) || set->count >= CP_MAX_SYMBOL_SET_ITEMS) {
        return 0;
    }
    snprintf(set->items[set->count++], sizeof(set->items[0]), "%s", symbol);
    return 1;
}

int cp_first_set_contains(const GrammarSymbolSet *set, const char *symbol) {
    return set_contains(set, symbol);
}

int cp_grammar_find_symbol(const GrammarSymbolTable *symbols, const char *name) {
    int i;
    if (symbols == NULL || name == NULL) {
        return -1;
    }
    for (i = 0; i < symbols->count; ++i) {
        if (strcmp(symbols->names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

static int add_symbol(GrammarSymbolTable *symbols, const char *name) {
    int index = cp_grammar_find_symbol(symbols, name);
    if (index >= 0) {
        return index;
    }
    if (symbols == NULL || name == NULL || symbols->count >= CP_MAX_GRAMMAR_SYMBOLS) {
        return -1;
    }
    snprintf(symbols->names[symbols->count], sizeof(symbols->names[0]), "%s", name);
    return symbols->count++;
}

static int grammar_has_token(const Grammar *grammar, const char *name) {
    int i;
    if (grammar == NULL || name == NULL) {
        return 0;
    }
    for (i = 0; i < grammar->token_count; ++i) {
        if (strcmp(grammar->tokens[i], name) == 0) {
            return 1;
        }
    }
    return strcmp(name, "EOF") == 0;
}

static int is_non_terminal(const Grammar *grammar, const char *name) {
    int i;
    if (grammar == NULL || name == NULL) {
        return 0;
    }
    for (i = 0; i < grammar->non_terminal_count; ++i) {
        if (strcmp(grammar->non_terminals[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

static void collect_symbols(const Grammar *grammar, GrammarSymbolTable *symbols) {
    int i;
    int j;
    if (grammar == NULL || symbols == NULL) {
        return;
    }
    memset(symbols, 0, sizeof(*symbols));
    for (i = 0; i < grammar->token_count; ++i) {
        add_symbol(symbols, grammar->tokens[i]);
    }
    add_symbol(symbols, "EOF");
    for (i = 0; i < grammar->non_terminal_count; ++i) {
        add_symbol(symbols, grammar->non_terminals[i]);
    }
    for (i = 0; i < grammar->production_count; ++i) {
        for (j = 0; j < grammar->productions[i].rhs_count; ++j) {
            add_symbol(symbols, grammar->productions[i].rhs[j]);
        }
    }
}

GrammarAnalysisResult cp_analyze_grammar(const Grammar *grammar) {
    GrammarAnalysisResult result;
    int changed = 1;
    int i;
    RESULT_SUCCESS(&result.base);
    memset(&result.data, 0, sizeof(result.data));
    if (grammar == NULL) {
        CompilerError error;
        cp_make_error(&error, 4001, "missing grammar for analysis", -1, -1, "yacc");
        RESULT_FAILURE(&result.base, error);
        return result;
    }

    collect_symbols(grammar, &result.data.symbols);
    for (i = 0; i < result.data.symbols.count; ++i) {
        if (grammar_has_token(grammar, result.data.symbols.names[i])) {
            set_add(&result.data.first[i], result.data.symbols.names[i]);
        }
    }

    while (changed) {
        changed = 0;
        for (i = 0; i < grammar->production_count; ++i) {
            int lhs_index = cp_grammar_find_symbol(&result.data.symbols, grammar->productions[i].lhs);
            int nullable_prefix = 1;
            int rhs_index = 0;
            if (lhs_index < 0) {
                continue;
            }
            if (grammar->productions[i].rhs_count == 0) {
                changed |= set_add(&result.data.first[lhs_index], CP_EPSILON_SYMBOL);
                continue;
            }
            while (nullable_prefix && rhs_index < grammar->productions[i].rhs_count) {
                int sym_index = cp_grammar_find_symbol(&result.data.symbols, grammar->productions[i].rhs[rhs_index]);
                int set_index;
                nullable_prefix = 0;
                if (sym_index < 0) {
                    break;
                }
                for (set_index = 0; set_index < result.data.first[sym_index].count; ++set_index) {
                    const char *item = result.data.first[sym_index].items[set_index];
                    if (strcmp(item, CP_EPSILON_SYMBOL) == 0) {
                        nullable_prefix = 1;
                    } else {
                        changed |= set_add(&result.data.first[lhs_index], item);
                    }
                }
                if (grammar_has_token(grammar, grammar->productions[i].rhs[rhs_index])) {
                    changed |= set_add(&result.data.first[lhs_index], grammar->productions[i].rhs[rhs_index]);
                    nullable_prefix = 0;
                }
                ++rhs_index;
            }
            if (nullable_prefix) {
                changed |= set_add(&result.data.first[lhs_index], CP_EPSILON_SYMBOL);
            }
        }
    }

    {
        int start_index = cp_grammar_find_symbol(&result.data.symbols, grammar->start_symbol);
        if (start_index >= 0) {
            set_add(&result.data.follow[start_index], "EOF");
        }
    }

    changed = 1;
    while (changed) {
        changed = 0;
        for (i = 0; i < grammar->production_count; ++i) {
            int lhs_index = cp_grammar_find_symbol(&result.data.symbols, grammar->productions[i].lhs);
            int j;
            for (j = 0; j < grammar->productions[i].rhs_count; ++j) {
                const char *symbol = grammar->productions[i].rhs[j];
                int sym_index = cp_grammar_find_symbol(&result.data.symbols, symbol);
                int nullable_suffix = 1;
                int k;
                if (sym_index < 0 || grammar_has_token(grammar, symbol)) {
                    continue;
                }
                for (k = j + 1; k < grammar->productions[i].rhs_count; ++k) {
                    int next_index = cp_grammar_find_symbol(&result.data.symbols, grammar->productions[i].rhs[k]);
                    int item_index;
                    nullable_suffix = 0;
                    if (next_index < 0) {
                        break;
                    }
                    for (item_index = 0; item_index < result.data.first[next_index].count; ++item_index) {
                        const char *item = result.data.first[next_index].items[item_index];
                        if (strcmp(item, CP_EPSILON_SYMBOL) == 0) {
                            nullable_suffix = 1;
                        } else {
                            changed |= set_add(&result.data.follow[sym_index], item);
                        }
                    }
                    if (grammar_has_token(grammar, grammar->productions[i].rhs[k])) {
                        changed |= set_add(&result.data.follow[sym_index], grammar->productions[i].rhs[k]);
                        nullable_suffix = 0;
                    }
                    if (!nullable_suffix) {
                        break;
                    }
                }
                if (j == grammar->productions[i].rhs_count - 1) {
                    nullable_suffix = 1;
                }
                if (nullable_suffix && lhs_index >= 0) {
                    int item_index;
                    for (item_index = 0; item_index < result.data.follow[lhs_index].count; ++item_index) {
                        changed |= set_add(&result.data.follow[sym_index], result.data.follow[lhs_index].items[item_index]);
                    }
                }
            }
        }
    }

    return result;
}
