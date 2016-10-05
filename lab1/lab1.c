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
    printf("Lab 1: start execution\n");
    float input, input2;
    scanf("%f", &input);
    scanf("%f", &input2);
    printf("expected (%f) %f\n", input + input2, single_float_add(input, input2));
    printf("exoected (3) %f\n", single_float_add(1.0, 2.0));
    printf("expected (7) %f\n", single_float_add(3.0, 4.0));
    //print_intfloat(&extracted);
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

    x->fraction = f_int & BITMASK_FRACTION;
    x->fraction |= BITMASK_HIDDEN_BIT;
    x->fraction = x->fraction << 7;

    f_int = f_int >> SIZE_FRACTION;
    x->exponent = f_int & BITMASK_EXPONENT;
    f_int = f_int >> SIZE_EXPONENT;
    x->sign = f_int & BITMASK_SIGN;

    // remove bias value from extracted exponent
    x->exponent -= EXPONENT_BIAS;
    if (x->sign)
        x->fraction *= -1;
}

/*
 * Normalize the INTFLOAT structure so that the exponent value is as small
 * as possible. This will simplify the arithmitic operations by making it
 * more easier to get the INTFLOATS on the same scale.
 */
void normalize(INTFLOAT_PTR x) {
    // Check for zero case
    if (x->fraction == 0)
        return;

    while (((x->fraction ^ (x->fraction << 1)) & BITMASK_BIT_31) == 0) {
        x->fraction = x->fraction << 1;
        x->exponent--;
    }
}

float single_float_add(float a, float b) {
    INTFLOAT a_if, b_if, result;
    extract_float(&a_if, a);
    extract_float(&b_if, b);

    if (a_if.exponent > b_if.exponent) {
        int diff = a_if.exponent - b_if.exponent;
        b_if.exponent += diff;
        b_if.fraction = b_if.fraction >> diff;       
    } else if (a_if.exponent < b_if.exponent) {
        int diff = b_if.exponent - a_if.exponent;
        a_if.exponent += diff;
        a_if.fraction = a_if.fraction >> diff;
    }
    
    a_if.fraction = a_if.fraction >> 1;
    b_if.fraction = b_if.fraction >> 1;
    a_if.exponent++;
    b_if.exponent++;

    result.exponent = b_if.exponent;
    result.fraction = b_if.fraction + a_if.fraction;
    result.sign = result.fraction < 0 ? 1 : 0;

    normalize(&result);

    return repack_intfloat(&result);
}

float single_float_subtract(float a, float b) {
    return single_float_add(a, b * -1);
}

float repack_intfloat(INTFLOAT_PTR x) {
    unsigned int f_int = 0;

    f_int |= x->sign;
    f_int = f_int << SIZE_EXPONENT;
    f_int |= x->exponent + EXPONENT_BIAS;
    f_int = f_int << SIZE_FRACTION;
    f_int |= (x->fraction >> 7) & BITMASK_FRACTION;

    return * (float*)&f_int;
}
