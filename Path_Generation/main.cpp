#include <stdio.h>

void Action1();
void Action2();
int NoP;
struct Coordinates
{
    float X;
    float Y;
};
int main(void)
{
    printf("How many points are in the path that you want to travel? \n");
    scanf ("%d", &NoP);
    int a;
    while (1){
        printf("Choose the number of one of the following:\n");
        printf("1) Define parametrically \n");
        printf("2) Enter an array of data \n");
        printf("3) Exit program \n");
        scanf("%d", &a);
        switch(a) {
            case 1 :
                Action1();
                break;
            case 2 :
                Action2();
                break;
            case 3 :
                return 0;
            default :
                printf("Invalid choice\n" );
        }
    }
}
void Action1()
{
    float t;
    float t_upper;
    float t_lower;
    char X;
    char Y;
    float difference;
    printf("Define t's bounds, and X and Y in terms of t. \n");
    printf("What is the upper bound of t? \n");
    scanf("%f", &t_upper);
    printf("What is the lower bound of t? \n");
    scanf("%f", &t_lower);
    printf("What is X in terms of t? \n");
    scanf("%s", &X);
    printf("What is Y in terms of t? \n");
    scanf("%s", &Y);
    struct Coordinates array[NoP];
    difference = t_upper - t_lower;
    float increment;
    increment = difference/(NoP-1);
    t = t_lower;
    int n;
    for(n=0; t <= t_upper; t= t + increment, n++){
        array[n].X = X;
        array[n].Y = Y;
        printf ("%f \n", t);
    }
    int i;
    int b;
    printf("\n\n Here is the resulting table:\n\n");
    for (i = 0; i < n; i++) {
        printf("X = %0.3f\t Y = %0.3f", array[i].X, array[i].Y);
        printf("\n");
    }
    printf("would you like to save this to a file? Enter the corresponding number.\n");
    printf("1) Yes \n");
    printf("2) No, I want to start over. \n");
    printf("3) Return to main menu. \n");
    scanf("%d", &b);
    switch(b) {
        case 1 :
        {
            FILE *file;
            file = fopen("Parametric_Path.txt", "wb");
            char* base = (char*) array;
            for(int j = 0; j < sizeof(array); j++){
                fputc(*(base+j), file);
            }
            fclose(file);
        }
            break;
        case 2 :
            Action1();
            break;
        case 3 :
            break;
        default :
            printf("Invalid choice\n" );
            Action1 ();
    }
}
void Action2() {
    int b;
    struct Coordinates array[NoP];
    printf("Enter the X coordinates in the table in increasing fashion. \n");
    int i;
    for (i = 0; i < NoP; i++) {
        scanf("%f", &array[i].X );
    }
    printf("Enter the Y coordinates in the order that corresponds to the order X's were entered. \n");
    for (i = 0; i < NoP; i++) {
        scanf("%f", &array[i].Y );
    }
    printf("\n\n Here are the values you entered:\n\n");
    for (i = 0; i < NoP; i++) {
        printf("X = %0.3f\t Y = %0.3f", array[i].X , array[i].Y );
        printf("\n");
    }
    printf("would you like to save this to a file? Enter the corresponding number.\n");
    printf("1) Yes \n");
    printf("2) No, I want to start over. \n");
    printf("3) Return to main menu. \n");
    scanf("%d", &b);
    switch(b) {
        case 1 :
        {
            FILE *file;
            file = fopen("Data_Array_Path.txt", "wb");
            char* base = (char*) array;
            for(int j = 0; j < sizeof(array); j++){
                fputc(*(base+j), file);
            }
            fclose(file);
        }
            break;
        case 2 :
            Action2();
            break;
        case 3 :
            break;
        default :
            printf("Invalid choice\n" );
            Action2 ();
    }
}

