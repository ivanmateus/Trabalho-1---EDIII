#include <stdio.h>
#include <stdlib.h>
#include "cityList.h"
#include <string.h>

cityNode *create_city_node(cityList *cities, char *city){
	cityNode *new = (cityNode *)malloc(sizeof(cityNode));	//Allocate memory for the node
	if(new == NULL){
		return 0;
	}
	new->city = (char *)malloc(strlen(city) * sizeof(char));
	strcpy(new->city,city);
	new->next = NULL;
	return new;
}

int check_for_city(char *city, cityList *cities){
	cityNode *aux = *cities;

	if(aux == NULL){
		cityNode *new = create_city_node(cities, city);
		*cities = new;
		return 1;
	}

	while(aux != NULL){
		if(!strcmp(city, aux->city)){
			return 0;
		}
		if(aux->next == NULL){
			cityNode *new = create_city_node(cities,city);
			aux->next = new;
			return 1;
		}
		aux = aux->next;
	}
	return 0;
}

