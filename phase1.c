#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define MAX_STR_SIZE 20
#define MAX_FILENAME_LEN 200

int compare_strings(const void *a, const void *b) {
	char *sa = (char *)a;
	char *sb = (char *)b;
	return strcmp(sa, sb);
}

void printArr (char arr[]){
	int i;
	for (i = 0 ; i <= strlen(arr) ;i++){    
			if (arr[i] == '\0'){
				printf("-"); 
			}if (arr[i] == '\n'){
				printf("$"); 
			}if (arr[i] == (char)3){
				printf("*"); 
			}else {
				printf("%c",arr[i]);
			}
		}
		printf("\n");
}

//Counting number of chars in input file
int countChars(int transform, FILE *fp){
	int counter =0;
	char ch;
	if (transform ==1){	//forward direction
		while ((ch = fgetc(fp)) != EOF){
			counter ++;
		}
		rewind (fp);
		return counter;
	}

	if (transform ==2){	//backward direction
		while ((ch = fgetc(fp)) != EOF){
			counter ++;
		}
		rewind (fp);
		return (counter-8);//ignores header

	}
	rewind (fp);
	return 0;
}

//Read input file and copy contents in data[][]
void readFile (int transform, int file_counter, int total_rows, FILE *fp, char data[total_rows][MAX_STR_SIZE+2]){
	int temp;
	int col = 0;
	int row = 0;

	if (transform == 1){ //forward
	    while (fread(&temp, sizeof(char), 1, fp) > 0) {
	        if (col < MAX_STR_SIZE){
	        	data[row][col] = temp;
	        	data[row][col+1] = (char)3;
	        	data[row][col+2] = (char)0;

	        } else{ 
	        	row ++;
	        	col = 0;
	        	data[row][col] = temp;   
	 	        data[row][col+1] = (char)3;
	        	data[row][col+2] = (char)0;
	       }       
	       col++;
	    }
	    int k = col+1; //filling reminder with zeros
		while (k != MAX_STR_SIZE+1){
			data[row][k] = '\0';
			k++;
		}
	}else { //backward transform
		int i = 0;
		while (fread(&temp, sizeof(char), 1, fp) > 0) {
		    if (col < MAX_STR_SIZE && i <= 7){ // Do nothing, chars for header = 7
		        i ++;
		        continue;
		    }else if(col <= MAX_STR_SIZE){
		        data[row][col] = temp;
		        data[row][col+1] = (char)0;
		    }else{ 
		        row ++;
		        col = 0;
		        data[row][col] = temp; 
		        data[row][col+1] = (char)0;
		    }
		    col++;
		    i++;
		}	
	    int k = col+1; //filling reminder with zeros
		while (k <= MAX_STR_SIZE+1){
			data[row][k] = '\0';
			k++;
		}

	}
}

void forwardTrans(int processingRow ,char data[], int total_rows, char forwRes[][MAX_STR_SIZE+1]){
	int dataLen = strlen(data);
	char rotations[dataLen][dataLen]; 
	int i,j;
	for (i = 0 ; i < dataLen+1 ; i++){
		for ( j =0 ; j < dataLen ; j++){
			rotations[i][j] = data[(j+i)%(dataLen)];	
			rotations[i][j+1] = (char)3;	
			rotations[i][j+2] = '\0';	
		}
		rotations[i][j+1] = (char)3;
		rotations[i][j+2] = '\0';
	}

	qsort(rotations, dataLen, sizeof(char) * dataLen, compare_strings);

	//FT - Resulting Forward Transform array
	for (j = 0 ; j < dataLen ; j++){
		forwRes[processingRow][j] = rotations[j][dataLen-1];
		}

	//FT - Filling reminder with zeros
	while (j != MAX_STR_SIZE+2){
		forwRes [processingRow][j] = '\0';
		j++;
	}
}


void backwardTrans(int processingRow, char data[], int total_rows, char backwardResult[][MAX_STR_SIZE+1]){
	int orgCol;
	int dataLen = strlen(data);
	
	char temp[dataLen][dataLen]; 
	char original[dataLen][dataLen]; 
	memset( temp, 0, dataLen*dataLen*sizeof(char) );
	memset( original, 0, dataLen*dataLen*sizeof(char) );

	int i;
  	for (i = 0 ; i < dataLen+1 ; i++){
  		original[i][0] = data[i];
  	}

  	int j;
  	for (j = 0 , orgCol =1; j <dataLen-1 ; j++, orgCol++){
	  	memcpy(temp, original, sizeof (char) * dataLen* dataLen);
		qsort(temp, dataLen, sizeof(char) * dataLen, compare_strings);
		for (i =0 ; i< dataLen ; i++ ){
			original[i][orgCol] = temp[i][orgCol-1];
		}
  	}

  	//testing for last char null in original
  	int string;
  	for (int i = 0 ; i < dataLen ; i++){
  		if (original[i][dataLen-1] == (char)3){
  			string = i;
  		}
  	}
  	//adding transformed string in resulting array
  	for (int i = 0 ; i < dataLen ; i++){
  		if (original[string][i] == (char)3){ //ignores EOT (char 3)
  			break;
  		}else {
  			backwardResult[processingRow][i] = original[string][i];
  		}
  	}
}


void writeOutfile(int transform, FILE *outfile, int file_counter, int total_rows, char result[total_rows][MAX_STR_SIZE+1]){
	if (transform == 1){ //forward trans
		//printing header
		fwrite("\xab",sizeof(char), 1, outfile); 
		fwrite("\xba",sizeof(char), 1, outfile); 
		fwrite("\xbe",sizeof(char), 1, outfile); 
		fwrite("\xef",sizeof(char), 1, outfile); 
		fwrite("\x14",sizeof(char), 1, outfile); 
		fwrite("\x00",sizeof(char), 1, outfile); 
		fwrite("\x00",sizeof(char), 1, outfile); 
		fwrite("\x00",sizeof(char), 1, outfile); 

		fwrite(result, sizeof(char), file_counter+(total_rows), outfile); 
	
	//removing null chars at end of each row of result[][]
	}else if (transform ==2){
		for (int i = 0 ; (i < total_rows) ; i ++ ){
			for (int j = 0; j < MAX_STR_SIZE+1 ; j++){

				if (result[i][j]  == (char) 0){
					continue;
				}else{
					char c = result[i][j];
					fprintf(outfile, "%c", c); 

				}
			}
		}		
	}
}

int main (int argc, char **argv){
    char filename[MAX_FILENAME_LEN] ;
	FILE *fp;
	FILE *outfile;
	int file_counter, total_rows; //temp, 
	int transform ;// 1 = forward | 2 = backward

	//Loop through command-line arguments
	int i;
    for (i = 1; i < argc; i++) {

		if ( (strcmp(argv[i], "--forward") == 0) || (strcmp(argv[i], "--backward") == 0)) {
			if ((strcmp(argv[i], "--forward") == 0) ){
				transform = 1;
				printf("\nProcessing forward transform...\n");
			}else{
				transform = 2;
				printf("\nProcessing backward transform...\n");

			}
        }
        if (strcmp(argv[i], "--infile") == 0) {
            strncpy(filename, argv[i+1], MAX_FILENAME_LEN);
			fp = fopen(filename, "r+");
			if (fp == NULL){
			 	fprintf(stderr, "Could not open %s for writing.\n", filename);
				return(1); //program exited due to error or anomaly	
			}
			printf("Processing input, %s\n",filename);

        }
        if (strcmp(argv[i], "--outfile") == 0) {
            strncpy(filename, argv[i+1], MAX_FILENAME_LEN);
            outfile = fopen(filename, "w");
        	if (outfile == NULL) {
				fprintf(stderr, "Could not open %s for writing.\n", filename);
				exit(1);
			}
			printf("Writting output to %s\n",filename);

        }
    }


//Forward trasnform 
    if (transform == 1){
	    file_counter = countChars(transform, fp);

	//FT - Reading input file
		float floati = ((float)file_counter / MAX_STR_SIZE); 
		total_rows = ceil(floati);
		char data[(total_rows)][MAX_STR_SIZE+2]; //+2 for null and EOT

		readFile(transform, file_counter, total_rows, fp, data);

	//FT - Creating resulting array	
		char forwardResult[total_rows][MAX_STR_SIZE+1] ; //array for post-sort
		for (int i = 0 ; i < total_rows ; i++){
			forwardTrans(i, data[i], total_rows, forwardResult);
		}

	//FT -Write header and resulting array in outfile		
		writeOutfile(transform, outfile, file_counter, total_rows, forwardResult);
	}

	
//header va de 0 a 7 inclusive
   	if (transform == 2){

	//BK - Reading input file
   		file_counter = (countChars(transform, fp));
		float floati = ((float)file_counter / MAX_STR_SIZE); 
		total_rows = ceil(floati);
		char data[(total_rows)][MAX_STR_SIZE+2]; 

		readFile(transform, file_counter, total_rows, fp, data);

	//BK - Creating resulting array	
		char backwardResult[total_rows][MAX_STR_SIZE+1] ;
		memset( backwardResult, 0, total_rows*(MAX_STR_SIZE+1)*sizeof(char) );
 
		for (int i = 0 ; i < total_rows ; i++){
			//printf("\n\nFT - SENDING %s\n\n", data[i]);
			backwardTrans(i, data[i], total_rows, backwardResult);
		}
		
		writeOutfile(transform, outfile, file_counter-2, total_rows, backwardResult);

    }

	printf("\nTransform complete, check output for result.\n");

    fclose(outfile);
    fclose(fp);
}