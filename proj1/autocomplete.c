#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "autocomplete.h"

// Compares two terms based on the memory address of their first character,
// which allows for lexicographic comparison.
int compare_terms(const void *p_a, const void *p_b) {
    term *p_a_t = (term *)p_a;
    term *p_b_t = (term *)p_b;
    return strcmp(p_a_t->term, p_b_t->term);
}

int compare_weights(const void *p_a, const void *p_b) {
    term *p_a_t = (term *)p_a;
    term *p_b_t = (term *)p_b;
    return p_b_t->weight - p_a_t->weight;
}

void read_in_terms(term **terms, int *pnterms, char *filename)
{
    // read in file
    char line[1000];
    FILE *fp = fopen(filename, "r");

    // read in first line (which has number of terms)
    fgets(line, sizeof(line), fp);
    *pnterms = atoi(line);

    // allocate memory for terms
    *terms = (term *)malloc(*pnterms * sizeof(term));

    // read in the rest of the lines into the terms array
    for(int i = 0; i < *pnterms; i++){
        fgets(line, sizeof(line), fp);
        char *temp = strtok(line, "\t");
       // printf("%f", atof(temp));
        (*terms)[i].weight = atof(temp);
        temp = strtok(NULL, "\n");
        strcpy((*terms)[i].term, temp);
    }

    // close the file
    fclose(fp);

    // sort the terms

    qsort(*terms, *pnterms, sizeof(term), compare_terms);

    /*
    // print out the terms
    for(int i = 0; i < *pnterms; i++){
        printf("%f %s\n", (*terms)[i].weight, (*terms)[i].term);
    } */
}

// checks if s2 is in s1
int check_s2_in_s1(const char* s1, const char* s2)
{
    int i = 0;
    while(s2[i] != '\0'){
        if(s2[i] != s1[i]){
            return s2[i]-s1[i];
        }
        i++;
    }
    return 0;
}


int lowest_match(term *terms, int nterms, char *substr)
{
    int low = 0;
    int high = nterms-1;
    int mid = 0;
    int final_mid = 0;
    int count = 0;
    while(low < high){
        mid = (low+high)/2;
        if((check_s2_in_s1((terms)[mid].term,substr)) == 0) //check if the substirng matches
        {
           // printf("here");
            final_mid = mid;
            low = high;
        }
        else if ((check_s2_in_s1((terms)[mid].term,substr)) > 0)// check if substring is greater than mid
        {
            low = mid + 1;
        }
        else{
            high = mid - 1;
        }
        count++;
        //printf("%d %d\n",low, high);
    }
    final_mid = mid;
    while ((check_s2_in_s1((terms)[final_mid].term,substr)) == 0) // the item at the index matches the substring
    {
        // iterate down the list until it doesnt match then return the lowest matching index
        final_mid--;
    }
    if(check_s2_in_s1((terms)[final_mid+1].term,substr) == 0){
        return final_mid + 1;
    }
    else{
        return -1;
    }
}


int highest_match(struct term *terms, int nterms, char *substr)
{
    int low = 0;
    int high = nterms-1;
    int mid = 0;
    int final_mid = 0;
    int count = 0;
    while(low < high){
        mid = (low+high)/2;
        if((check_s2_in_s1((terms)[mid].term,substr)) == 0) //check if the substirng matches
        {
           // printf("here");
            final_mid = mid;
            low = high;
        }
        else if ((check_s2_in_s1((terms)[mid].term,substr)) > 0)// check if substring is greater than mid
        {
            low = mid + 1;
        }
        else{
            high = mid - 1;
        }
        count++;
        //printf("%d %d\n",low, high);
    }
    final_mid = mid;
    while ((check_s2_in_s1((terms)[final_mid].term,substr)) == 0) // the item at the index matches the substring
    {
        // iterate up the list until it doesnt match then return the highest matching index
        final_mid++;
    }
    if(check_s2_in_s1((terms)[final_mid-1].term,substr) == 0){
        return final_mid - 1;
    }
    else{
        return -1;
    }
}

void autocomplete(term **answer, int *n_answer, term *terms, int nterms, char *substr)
{
    // find the highest and lowest index that matches
    int max_ind = highest_match(terms, nterms, substr);
    int min_ind = lowest_match(terms, nterms, substr);
    if(max_ind == min_ind)
        *n_answer = 0;
    else
        *n_answer = max_ind - min_ind + 1; // the number of answers is the highest index - lowest index

    // create an array of terms for answers
    *answer = (term *)malloc(*n_answer * sizeof(term));

    // iterate through terms from lowest index to highest and put the values in answer
    int count = 0;
    for(int i = min_ind; i <= max_ind; i++){
       // printf("%f %s",terms[i].weight,terms[i].term);
        (*answer)[count].weight = (terms)[i].weight;
        strcpy((*answer)[count].term, (terms)[i].term);
        count ++;
    }

    // sort answer by weight in non-increasing order
    qsort(*answer, *n_answer, sizeof(term), compare_weights);
}

int main(void)
{
    struct term *terms;
    int nterms;
    read_in_terms(&terms, &nterms, "wiktionary.txt");
    lowest_match(terms, nterms, "Tor");
    highest_match(terms, nterms, "Tor");
    
    struct term *answer;
    int n_answer;
    autocomplete(&answer, &n_answer, terms, nterms, "ele");
    for(int i = 0; i < n_answer; i++){
        printf("%f, %s\n",answer[i].weight,answer[i].term);
    }
    //free allocated blocks here -- not required for the project, but good practice
    return 0;
}
