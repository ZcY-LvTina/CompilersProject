int inc(int x) {
    return x + 1;
}

int main() {
    int sum = 1 + 2;
    while (sum < 5) {
        sum = inc(sum);
    }
    if (sum >= 5) {
        sum = sum - 1;
    } else {
        sum = 0;
    }
    return sum;
}
