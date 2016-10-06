#include <stdio.h>

#define EXPONENT_BIAS 127

#define BITMASK_SIGN 1
#define SIZE_SIGN 1

#define BITMASK_EXPONENT 0xFF
#define SIZE_EXPONENT 8

#define BITMASK_FRACTION 0x7FFFFF
#define SIZE_FRACTION 23
#define BITMASK_HIDDEN_BIT 0x800000

#define BITMASK_BIT_31 0x80000000

typedef struct _intfloat {
    int sign;
    int fraction;
    int exponent;
} INTFLOAT, *INTFLOAT_PTR;

void print_intfloat(INTFLOAT_PTR x);
void extract_float(INTFLOAT_PTR x, float f);
void normalize(INTFLOAT_PTR x);
float single_float_add(float a, float b);
float single_float_subtract(float a, float b);
float repack_intfloat(INTFLOAT_PTR x);

int main() {
    printf("Lab 1: input two numbers to be added and subtracted\n");
    float input, input2;
    printf("First num: ");
    scanf("%f", &input);
    printf("Second num: ");
    scanf("%f", &input2);
    printf("Add expected (%f) %f\n", input + input2, single_float_add(input, input2));
    printf("Sub expected (%f) %f\n", input - input2, single_float_subtract(input, input2));
}

void print_intfloat(INTFLOAT_PTR x) {
    printf("sign:     %d\n", x->sign);
    printf("fraction: %d\n", x->fraction);
    printf("exponent: %d\n", x->exponent);
}

/*
 * Extract 3 seperate fields from IEEE 754 single precision number
 * Fields starting from MSB:
 * Sign: 1 bit
 * Exponent: 8 bits
 * Fraction: 23 bits
 *
 * - Removes bias value from exponent field
 * - Adds hidden '1' in 24th bit position of fraction field
 */
void extract_float(INTFLOAT_PTR x, float f) {
    unsigned int f_int = * (unsigned int*)&f;

    // Extract fraction from float    
    x->fraction = f_int & BITMASK_FRACTION;
    x->fraction |= BITMASK_HIDDEN_BIT;
    x->fraction = x->fraction << 7;
    f_int = f_int >> SIZE_FRACTION;

    // Extract exponent from float
    x->exponent = f_int & BITMASK_EXPONENT;
    f_int = f_int >> SIZE_EXPONENT;
    // Adjust for exponent bias
    x->exponent -= EXPONENT_BIAS;

    // Extract sign
    x->sign = f_int & BITMASK_SIGN;

    // Adjust fraction value with sign to add correctly
    if (x->sign)
        x->fraction *= -1;
}

/*
 * Normalize the INTFLOAT structure
 * This will shift the number as far left as possible and update the exponent
 * accordingly 
 */
void normalize(INTFLOAT_PTR x) {
    // Check for zero case
    if (x->fraction == 0)
        return;

    // while the two MSB's of x->fraction are the same shift left and adjust
    // exponent
    while (((x->fraction ^ (x->fraction << 1)) & BITMASK_BIT_31) == 0) {
        x->fraction = x->fraction << 1;
        x->exponent--;
    }
}

/*
 * Adds two float numbers
 *  - extracts both to intfloats
 *  - gets both intfloats to same scale
 *  - account for overflow
 *  - exponental add
 *  - normalize
 *  - repack and return
 */
float single_float_add(float a, float b) {
    INTFLOAT a_if, b_if, result;
    extract_float(&a_if, a);
    extract_float(&b_if, b);

    // Get both intfloats on the same scale
    if (a_if.exponent > b_if.exponent) {
        int diff = a_if.exponent - b_if.exponent;
        b_if.exponent += diff;
        b_if.fraction = b_if.fraction >> diff;       
    } else if (a_if.exponent < b_if.exponent) {
        int diff = b_if.exponent - a_if.exponent;
        a_if.exponent += diff;
        a_if.fraction = a_if.fraction >> diff;
    }

    // Shift both intfloats right to account for possible overflow
    a_if.fraction = a_if.fraction >> 1;
    b_if.fraction = b_if.fraction >> 1;
    a_if.exponent++;
    b_if.exponent++;

    // Do addition
    result.exponent = b_if.exponent;
    result.fraction = b_if.fraction + a_if.fraction;
    result.sign = result.fraction < 0 ? 1 : 0;

    normalize(&result);

    return repack_intfloat(&result);
}

/*
 * Subtracts two float numbers
 *  - uses addition routine
 */
float single_float_subtract(float a, float b) {
    // Add the oppisite
    return single_float_add(a, b * -1);
}

/*
 * Take intfloat and pack values back into IEEE 754 float format
 */
float repack_intfloat(INTFLOAT_PTR x) {
    unsigned int f_int = 0;

    f_int |= x->sign;
    f_int = f_int << SIZE_EXPONENT;
    f_int |= x->exponent + EXPONENT_BIAS;
    f_int = f_int << SIZE_FRACTION;
    f_int |= (x->fraction >> 7) & BITMASK_FRACTION;

    return * (float*) &f_int;
}
