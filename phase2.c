#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

#define ENCODE_MODE 0
#define DECODE_MODE 1

void usage() {
    fprintf(stderr, "usage: phase2 [--encode|--decode] --infile <filename> --outfile <filename>\n");
}

void print_list(charval_t *list) {
	for ( ; list != NULL ; list = list->next){
		if (list->flag == 0){
			printf ("%d.%c,", list->val, list->c);

		}else if (list->flag ==1){ 	//char only
			printf ("%c,", list->c);

		}else {	// int only
			printf ("%d,", list->val);

		}
	}
	printf ("\n");
}


//Method will retrieve blocksize and chars from input file, saves info in char *data.
char* read_file(int mode, FILE * infile, int *blocksize, int* dataLen){
	assert (infile != NULL);

	char *data;
	char header[4];
	fread(&header, sizeof(char), 4, infile); //ignores header
	fread(blocksize, sizeof(char), 4, infile); 

	int init = 5; //arbitrary size to initialize malloc array
	data = calloc(init, sizeof(char));
	if (data == NULL){
		fprintf(stderr, "read_file - malloc of %d bytes failed", init);
		exit(1);
	}

	int temp;
	int i=0;
	while (fread(&temp, sizeof(char),1 , infile) > 0) {
		if (i >= init){ //testing if data is full
			init *= 2;
			data = realloc(data, init);
		}
		data[i] = temp;
		i++;
	}
	data[i] = '\0';
	*dataLen = i;
    fclose(infile);

	return data;
}

charval_t* mtf_encode(char *data, int dataLen){
	// Checking if data char has alrady been added to list
	charval_t* list = NULL;
	charval_t* list_encoding = NULL;
	int list_counter = 1;
	for (int i = 0; i <= dataLen-1 ; i++){
		int found = 0; // 0 == false == not found
		
		for (charval_t *curr = list; curr != NULL ; curr = curr->next){
			/*If character has already appeared, then we find its position in the list, output
			* that position, and move the char to the front of the list
			*/
			if (curr->c == data[i]){ //char already in list
				found = 1;

				int val = find_val(list, (char)data[i]);
				charval_t* buffer = new_charval((char)0, val, 2); // (2)flag for int only
				list_encoding = add_end(list_encoding, buffer);

				list = find_remove(list, curr->c);
				list = add_front(list, curr);
				update_val(list);

				break;
			}
		}
		/*When a new char appears in the input we output the code corresponding
		*to the first unused position in the list and follow this with the char.
		*/
		if (found == 0){ //char not found on list, must be added
			charval_t *temp = new_charval(data[i], list_counter, 0); // (0)flag for int and val
			list = add_end(list, temp);
			update_val(list);
			list_counter ++;

			int val = find_val(list, (char)data[i]);
			charval_t* buffer_val = new_charval((char)0, val, 2); // (2) flag for int only
			list_encoding = add_end(list_encoding, buffer_val);
			charval_t* buffer_char = new_charval(data[i], 0, 1); // (1) flag for char only
			list_encoding = add_end(list_encoding, buffer_char);
		}
	}
	return list_encoding;
}

int is_next_one(charval_t* curr, int counter, charval_t* i, int* repeats){
	if ( (curr->val == 1) && (curr->flag == 2) && (curr->next != NULL)){
		i = i->next;
		return is_next_one(curr->next, 1+counter, i, repeats);
	}
	*repeats = counter;
	return 0;
}


charval_t* run_length_encoding(charval_t* encoding){
	assert (encoding != NULL);

	charval_t *rle = NULL;
	charval_t *j;
	charval_t *k; //leading pointer

	int repeats;
	int rep_flag;
	for ( charval_t *i = encoding; i != NULL ; i=i->next ){
		rep_flag = 0;
		if ((i->val == 1) && (i->flag == 2) && (i->next != NULL)){
			j = i->next;
			if ((j->val == 1) && (j->flag == 2) && (j->next != NULL) ){
				k = j->next;
				is_next_one(k, 2, i, &repeats);

				if (repeats > 2){ 
					rep_flag = 1;
					
				}
			}
		}
		if (rep_flag){
			charval_t* zero = new_charval((char)0, 0, 2); 
			charval_t* reps = new_charval((char)0, repeats, 2); 
			rle = add_end(rle, zero);
			rle = add_end(rle, reps);

			for (int j = 1 ; j < repeats ; j++ ){
				i = i->next;
			}	
		}else{
			if (i->flag == 1){ // FLAG  0 = char & val, 1 = char, 2 = int
				charval_t* temp = new_charval(i->c, 0, 1);
				rle = add_end(rle, temp);
			}else{
				charval_t* temp = new_charval((char)0, i->val, 2);
				rle = add_end(rle, temp);
			}
		}
	}
	return rle;
}

charval_t* run_length_decode (charval_t* list){
		charval_t* rld = NULL;
		int skip = 0;
		for ( ; list != NULL ; list = list->next){ //0 = char & val, 1 = char, 2 = int
			if (skip == 1){
				skip = 0;
				continue;
			}
			if ((list->flag == 2) && (list->val == 0)){ //int
				charval_t* i = list->next;
				for ( int j = 0 ; j < i->val ; j++ ){
					charval_t* buffer = new_charval((char)0, 1, 2);
					rld = add_end(rld, buffer);
					skip = 1;
				}
			}else if (list->flag == 2){
				charval_t* buffer = new_charval((char)0, list->val, 2);
				rld = add_end(rld, buffer);
			} else{
				charval_t* buffer = new_charval(list->c, 0, 1);
				rld = add_end(rld, buffer);
			}
		}
		return rld;
}


charval_t* from_ascii (char* data, int dataLen){
	assert (data != NULL);
	assert (dataLen != 0);

	charval_t* stringy = NULL;
	for (int i = 0 ; i < dataLen ; i++){
		if ((data[i]+128 >= 128) && (data[i]+128 <= 255)){
			charval_t* buffer = new_charval(data[i], 0, 1);
			stringy = add_end(stringy, buffer);
		}else{
			charval_t* buffer = new_charval((char)0, data[i]+128, 2);
			stringy = add_end(stringy, buffer);
		}
	}

	return stringy;
}

void to_write(charval_t* list, FILE *outfile, int mode, int blocksize){
	assert (list != NULL);
	assert (outfile != NULL);
	assert ((mode == 1) || (mode == 0));

	if (mode == 0){
		fwrite("\xda",sizeof(char), 1, outfile); 
		fwrite("\xaa",sizeof(char), 1, outfile); 
		fwrite("\xaa",sizeof(char), 1, outfile); 
		fwrite("\xad",sizeof(char), 1, outfile); 

	}else {
		fwrite("\xab",sizeof(char), 1, outfile); 
		fwrite("\xba",sizeof(char), 1, outfile); 
		fwrite("\xbe",sizeof(char), 1, outfile); 
		fwrite("\xef",sizeof(char), 1, outfile); 
	}
	fwrite(&blocksize, sizeof(int), 1, outfile); 

	for ( ; list != NULL ; list = list->next){
		if (list->flag == 1){ //1 = char
			fprintf(outfile, "%c", list->c); 
		}else{
			fprintf(outfile, "%c", list->val+128); 
		}
	}
	fclose(outfile);
}

charval_t* mtf_decode (charval_t* rld){
	assert (rld != NULL);

	charval_t* encoding = NULL;
	charval_t* char_list = NULL;
	charval_t* buffer;

	int skip_next_round = 0;
	int counter = 0;
	for (charval_t* i = rld; i != NULL ; i = i->next, counter++){
		if (skip_next_round == 1){
			skip_next_round = 0;
			continue;
		}
		// FLAG -  0 = char & val,	 1 = char, 	2 = int
		if ((i->next != NULL) && (i->next->flag == 1 ) && (i->flag == 2)){
			buffer = new_charval(i->next->c, 0, 1);
			char_list = add_end(char_list, buffer);
			update_val(char_list);

			buffer = new_charval(i->next->c, 0, 1);
			encoding = add_end(encoding, buffer);

			skip_next_round = 1;

		}else{
			char foundy = find_char(char_list, i->val);
			buffer = new_charval(foundy,0,1);
			encoding = add_end(encoding, buffer);

			char_list = find_remove(char_list, foundy);
			charval_t* front = new_charval(foundy, 0, 1);
			char_list = add_front(char_list, front);
		}
	}


	return encoding;

}

int main(int argc, char *argv[]) {
    int c, mode, blocksize;
    char *infile_name = NULL;
    char *outfile_name = NULL;
    int encode_flag = 0;
    int decode_flag = 0;

// Argument parser, based on http://bit.ly/2tHBpo1
    for (;;) {
    	// ????? struct
        static struct option long_options[] = {
            {"encode",     no_argument,       0, 'e'},
            {"decode",     no_argument,       0, 'd'},
            {"infile",     required_argument, 0, 'i'},
            {"outfile",    required_argument, 0, 'o'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long (argc, argv, "efi:o:",long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 'i':
            infile_name = optarg;
            break;
        case 'o':
            outfile_name = optarg;
            break;
        case 'e':
            encode_flag = 1;
            break;
        case 'd':
            decode_flag = 1;
            break;
        default:
            fprintf(stderr, "shouldn't be here...");
            assert(0);
        }
    }

    if (encode_flag == 0 && decode_flag == 0) {
        usage();
        exit(1);
    } else if (encode_flag == 1 && decode_flag == 1) {
        fprintf(stderr, "usage: choose one of --decode or --encode\n");
        exit(1);
    } else if (encode_flag == 1) {
        mode = ENCODE_MODE;
    } else if (decode_flag == 1) {
        mode = DECODE_MODE; 
    } else {
        fprintf(stderr, "shouldn't be here...\n");
        assert(0);
    }

    if (infile_name == NULL) {
        usage();
        exit(1);
    }
        
    if (outfile_name == NULL) {
        usage();
        fprintf(stderr, "%s: need --outfile <filename>\n", argv[0]);
        exit(1);
    }

// Open input and outfiles
    FILE *infile;
    FILE *outfile;

	infile = fopen(infile_name, "r+");
	if (infile == NULL){
		fprintf(stderr,"Could not open infile %s for writting.\n", infile_name);
		exit(1);
	}
	outfile = fopen(outfile_name, "w");
	if (outfile == NULL){
		fprintf(stderr,"Could not open outfile %s for writting.\n", outfile_name);
		exit(1);
	}

//Encode Transform
	if (mode == 0){
		int dataLen =0; 
		char* data = read_file(mode, infile, &blocksize, &dataLen);
		charval_t* encoding = mtf_encode(data, dataLen);  
		charval_t* rle = run_length_encoding(encoding);
		to_write(rle, outfile, mode, blocksize);

		//freeing memory
		free_list(encoding);
		free_list(rle);

//Decode transform
	} else{
		int dataLen =0; 
		char* data = read_file(mode, infile, &blocksize, &dataLen);
		charval_t* rdata = from_ascii(data, dataLen);
		charval_t* rld = run_length_decode(rdata);
		charval_t* mtf = mtf_decode(rld);
		to_write(mtf, outfile, mode, blocksize);

		//freeing memory
		free_list(rdata);
		free_list(rld);
		free_list(mtf);
	}
}