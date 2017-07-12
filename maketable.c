#include <stdio.h>
#include <stdlib.h>

/* 1) read in a volt_spacing value from the user
 * 2) use this volt_spacing value to calculate col of necessary array
 * 3) create struct consisting of whatever variables the arduino reads out the velocity as
 * 4) create float (maybe double, long) array of necessary col
 * 	how many rows?		1
 * 	how many columns?	however many measurements are necessary
 *
 * 	the float array will be 1-d, but it will be made up of structs 
 * 5) create a for loop to fill in volt values, wait a pre-determined time, and read velocity/ang-velocity measurements from sensors
 * 	after each measurement, do not continue until prompted by user
 * 	assume that the boat will be reset to starting position after each measurement
 * 	include option to break manually if desired
 * 6) when entire array is filled out, save array as text file in current directory
 * 7) print to screen that the job is done */



// This function prompts the user to input a voltage spacing, and returns the necessary size of the data table (an array of sthuctures).
int getSize()
{
	double volt_spacing, volt_spacing_x100, max_volt_x100;			// read in volt_spacing value

	printf("Please input a value for volt_spacing: ");
	scanf("%lf", &volt_spacing);
	getchar();
	printf("\n");

	printf("volt_spacing equals %lf \n", volt_spacing);
	while(volt_spacing < 0.1)						// safeguard against accidental input or input that's less than 0.1
	{
		printf("Faulty input for volt_spacing, please input another value: ");
		scanf("%lf", &volt_spacing);
		getchar();
		printf("\n");
	}

	int col;							// use volt_spacing value to calculate # of columns in array 
	max_volt_x100 = 200;
	volt_spacing_x100 = volt_spacing * 100;
	col = (int) max_volt_x100 / volt_spacing_x100 + 1;
	printf("number of columns is %d\n\n", col);




	return col;
}





// This function creates a struct array of given size, then fills it out with the data.
// Then, it prints out the data to a text file called "data.txt".
void dataFill(int col)
{

	struct data							// creates structure type "data" which contains the 4 data values
	{
		float volt, del_volt, vel, ang;
	};

	struct data dataset[col];					// creates an array of "data" structs

	int i;
	char kill;


	// fills out struct array with value of the current column # - 20.271, as a test. the -20.271 was added to test negative float values.
	for (i = 0; i < col; i++)
	{
		dataset[i].volt = dataset[i].del_volt = dataset[i].vel = dataset[i].ang = i - 20.271;



		printf("To kill the program, type k and hit enter. To keep going, press enter.\n");

		kill = getchar();
		if (kill == 'k')
		{
			printf("program is killed.\n\n");
			break;
		}
	}




	FILE *f = fopen("data.txt", "w");								// opens a file "data.txt" to write to

	// prints out header for struct array, and also to a file data.txt
	printf("Voltage		Velocity		delta_Voltage	Angular velocity\n");
	fprintf(f, "Voltage		Velocity		delta_Voltage	Angular velocity\n");

	// prints out struct array to screen, and also to a file data.txt

	for (i = 0; i < col; i++)
	{
		printf("%lf	", dataset[i].volt);
		printf("%lf		", dataset[i].vel);
		printf("%lf	", dataset[i].del_volt);
		printf("%lf	\n", dataset[i].ang);
	
		fprintf(f, "%lf	", dataset[i].volt);
		fprintf(f, "%lf		", dataset[i].vel);
		fprintf(f, "%lf	", dataset[i].del_volt);
		fprintf(f, "%lf	\n", dataset[i].ang);
	
	};

	fclose(f);											// closes stream 

	printf("\n\n");

	printf("The above data has also been saved in a file called \"data.txt\", which should be in the same directory that this executable file is in.\n");
	printf("See the file \"tableREADME.txt\" for information about how the table is laid out.\n\n");
}





int main()
{
	int col = getSize();

	dataFill(col);

	return 0;
}
