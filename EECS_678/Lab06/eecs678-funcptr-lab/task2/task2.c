#include <stdio.h>
#include <stdlib.h>

typedef int (*operation[4]) (int a, int b);

/* IMPLEMENT ME: Declare your functions here */
int add (int a, int b);
int subtract (int a, int b);
int multiply (int a, int b);
int divide (int a, int b);

int main (void)
{
	/* IMPLEMENT ME: Insert your algorithm here */
	int a = 10;
	int b = 5;
	int input;

	operation math;

	math[0] = add;
	math[1] = subtract;
	math[2]	= multiply;
	math[3]	= divide;

	printf("Operand 'a' : %d | Operand 'b' : %d\n", a, b);
	printf("Specify the operation to perform (0 : add | 1 : subtract | 2 : Multiply | 3 : divide): ");
	scanf("%d", &input);

	int x = math[input] (a, b);
	printf("x = %d", x);
	printf("\n");

	return 0;
}

/* IMPLEMENT ME: Define your functions here */
int add (int a, int b) { printf ("Adding 'a' and 'b'\n"); return a + b; }
int subtract (int a, int b) {printf ("Subtracting 'a' from 'b'\n"); return a - b; }
int multiply (int a, int b) {printf ("Multiplying 'a' and 'b'\n"); return a * b; }
int divide (int a, int b) {printf ("Dividing 'a' by 'b'\n"); return a / b; }
