#ifndef CITYLIST_H_
#define CITYLIST_H_
#include <stdio.h>
#include <stdlib.h>

//-----Data structure for a node of a list of cities
typedef struct cityNode {
	char *city;
	struct cityNode* next;
} cityNode;

//-----Data structure for a list of cities
typedef struct cityNode* cityList;

cityNode *create_city_node(cityList *cities, char *city);
int check_for_city(char *city, cityList *cities);

#endif
