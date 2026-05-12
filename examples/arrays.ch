// arrays.ch — Array operations demo
// Demonstrates: int[], float[], char[] arrays

polamanna

main() {
    // Integer array
    int[] nums;
    nums[0] = 10;
    nums[1] = 20;
    nums[2] = 30;
    nums[3] = 40;
    nums[4] = 50;

    "=== Integer Array ===" sollu
    "nums[0] = " + nums[0] sollu
    "nums[1] = " + nums[1] sollu
    "nums[2] = " + nums[2] sollu
    "nums[3] = " + nums[3] sollu
    "nums[4] = " + nums[4] sollu

    // Sum the array
    int sum;
    sum = 0;
    int i;
    i = 0;
    while (i < 5) {
        sum = sum + nums[i];
        i = i + 1;
    }
    "Sum of nums = " + sum sollu

    // Float array
    float[] prices;
    prices[0] = 9.99;
    prices[1] = 19.50;
    prices[2] = 4.75;

    "=== Float Array ===" sollu
    "prices[0] = " + prices[0] sollu
    "prices[1] = " + prices[1] sollu
    "prices[2] = " + prices[2] sollu

    // Char array
    char[] letters;
    letters[0] = 'H';
    letters[1] = 'i';
    letters[2] = '!';

    "=== Char Array ===" sollu
    "letters = " + letters sollu
}

niruthuanna
