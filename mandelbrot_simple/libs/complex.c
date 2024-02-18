#include "complex.h"
#include <stdio.h>

void complex_to_str(char* dest_str, complex number)
{
    sprintf(dest_str, "%.2f + %.2fi", number.re, number.im);
}

complex complex_mult(complex a, complex b)
{
    double out_re = a.re * b.re - (a.im * b.im);
    double out_im = a.im * b.re + a.re * b.im;

    complex out;
    out.re = out_re;
    out.im = out_im;

    return out;
}

complex complex_add(complex a, complex b)
{
    complex out;
    out.re = a.re + b.re;
    out.im = a.im + b.im;

    return out;
}

complex complex_arithmetic(char comp_str[MAX_COMPUTATION_STR_LEN], complex* values)
{
    complex value_stack[MAX_STACK_LEN];
    char operation_stack[MAX_STACK_LEN];
    int depth = 0;
    int value_stack_offset = -1, operation_stack_offset = -1;

    for (int i = 0; i < strlen(comp_str); i++)
    {
        if (comp_str[i] == '(')
        {
            depth++;
        }
        else if (comp_str[i] == ')')
        {
            depth--;
            if (operation_stack_offset != -1 && operation_stack_offset == depth)
            {
                complex result = { 0, 0 };

                result = value_stack[value_stack_offset];
                value_stack_offset--;

                switch (operation_stack[operation_stack_offset])
                {
                case '+':
                    result = complex_add(result, value_stack[value_stack_offset]);
                    break;
                case '*':
                    result = complex_mult(result, value_stack[value_stack_offset]);
                    break;
                }

                value_stack_offset--;
                operation_stack_offset--;

                value_stack_offset++;
                value_stack[value_stack_offset] = result;
            }
        }
        else if (comp_str[i] == '+' || comp_str[i] == '*')
        {
            operation_stack_offset++;
            operation_stack[operation_stack_offset] = comp_str[i];
        }
        else if ((comp_str[i] >= 'a' && comp_str[i] <= 'z'))
        {
            int value_index = comp_str[i] - 'a';
            value_stack_offset++;
            value_stack[value_stack_offset] = values[value_index];
            if (operation_stack_offset != -1 && operation_stack_offset == depth)
            {
                complex result = { 0, 0 };

                result = value_stack[value_stack_offset];
                value_stack_offset--;

                switch (operation_stack[operation_stack_offset])
                {
                case '+':
                    result = complex_add(result, value_stack[value_stack_offset]);
                    break;
                case '*':
                    result = complex_mult(result, value_stack[value_stack_offset]);
                    break;
                }

                value_stack_offset--;
                operation_stack_offset--;

                value_stack_offset++;
                value_stack[value_stack_offset] = result;
            }
        }
        else if(comp_str[i] == ' ')
        {
            // skip
        }
        else
        {
            printf("Invalid symbol in the computation string!\n");
        }
    }

    return value_stack[value_stack_offset];
}
