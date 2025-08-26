#include "../libs/ck_internal.h"
#include "../libs/ck.h"

HashMap *signal_map = NULL;

void signal_connect(void *sender, enum SIGNAL signal, SignalHandler func, void *data) {
	if (!sender || !func) return;
	if (!signal_map) signal_map = hashmap_create(128);
	SignalManager *manager = hashmap_get(signal_map, (long long int)sender);
	if (!manager) {
		manager = malloc(sizeof(SignalManager));
		if (!manager) {
			fprintf(stderr, "Failed to allocate memory for SignalManager\n");
			return;
		}
		manager->signal = signal;
		manager->handler = func;
		manager->data = data;
		manager->next = NULL;
		if (hashmap_insert(signal_map, (long long int)sender, (void *)manager))
			printf("insert failed\n");
	} else {
		SignalManager *current = manager;
		while(current->next) {
			if (current->signal == signal) {
				current->handler = func;
				current->data = data;
				return;
			}
			current = current->next;
		}
		SignalManager *new_manager = malloc(sizeof(SignalManager));
		if (!new_manager) {
			fprintf(stderr, "Failed to allocate memory for new SignalManager\n");
			return;
		}
		new_manager->signal = signal;
		new_manager->handler = func;
		new_manager->data = data;
		new_manager->next = NULL;
		current->next = new_manager;
	}
}

void signal_disconnect(void *sender, enum SIGNAL signal, SignalHandler func) {
	if (!sender || !func || !signal_map) return;
	SignalManager *manager = hashmap_get(signal_map, (long long int)sender);
	if (!manager) return;
	SignalManager *current = manager;
	SignalManager *prev = NULL;
	while (current) {
		if (current->signal == signal) {
			if (prev) {
				prev->next = current->next;
				free(current);
			} else {
				hashmap_replace(signal_map, (long long int)sender, current->next);
				free(current);
			}
			return;
		}
		prev = current;
		current = current->next;
	}
}

void signal_emit(void *sender, enum SIGNAL signal) {
	if (!signal_map) return;
	SignalManager *manager = hashmap_get(signal_map, (long long int)sender);
	if (!manager) return;

	while (manager) {
		if (manager->signal == signal) {
			manager->handler(sender, manager->data);
			return;
		}
		manager = manager->next;
	}
}