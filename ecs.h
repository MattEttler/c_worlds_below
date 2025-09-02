#ifndef ECS_H
#define ECS_H

#include <stdint.h>

typedef int32_t Entity;
#define MAX_ENTITY_COUNT 1000

// TODO: Make a static "garbage_bin" of deleted entity Ids that can be reused.
// Debug assertions should frequently check to ensure that entities are not
// "leaking" components.

#define COMPONENT(ComponentName, DataType)					\
typedef struct {								\
	DataType data[MAX_ENTITY_COUNT];					\
	Entity entities[MAX_ENTITY_COUNT];					\
	Entity entity_index[MAX_ENTITY_COUNT];				\
	Entity count;								\
} ComponentName;								\
										\
void add_##ComponentName(ComponentName* comp, Entity e, DataType value) { 	\
	comp->entity_index[e] = comp->count;					\
	comp->data[comp->count] = value;					\
	comp->entities[comp->count] = e;					\
	comp->count++;								\
}										\
										\
DataType* get_##ComponentName(ComponentName* comp, Entity e) {			\
	Entity idx = comp->entity_index[e];					\
	if (idx < comp->count && idx > -1 && comp->entities[idx] == e) {	\
		return &comp->data[idx];					\
	}									\
	return NULL;								\
}										\
										\
void remove_##ComponentName(ComponentName* comp, Entity e) {			\
    	size_t idx = comp->entity_index[e];                                     \
    	size_t last_idx = comp->count - 1;                                      \
	comp->data[idx] = comp->data[last_idx];                         	\
	Entity last_entity = comp->entities[last_idx];                  	\
	comp->entities[idx] = last_entity;                              	\
	comp->entity_index[last_entity] = idx;                          	\
	comp->count--;                                                          \
}

#endif // ECS_H
