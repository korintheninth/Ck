#include "../libs/ck.h"
#include "../libs/ck_internal.h"

HashMap *hashmap_create(size_t size) {
	HashMap *map = calloc(1, sizeof(HashMap));
	if (!map) {
		fprintf(stderr, "Failed to allocate memory for HashMap\n");
		return NULL;
	}
	map->size = size;
	map->count = 0;
	map->buckets = calloc(size, sizeof(Bucket *));
	if (!map->buckets) {
		fprintf(stderr, "Failed to allocate memory for HashMap buckets\n");
		free(map);
		return NULL;
	}
	return map;
}

static inline size_t sdbm(const char *str) {
	size_t hash = 0;
	int c;
	while ((c = *str++)) {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}

int hashmap_insert(HashMap *map, long long int key, void *value) {
	if (!map || !map->buckets) {
		fprintf(stderr, "HashMap is not initialized\n");
		return -1;
	}
	size_t index = sdbm((const char *)&key) % map->size;
	Bucket *bucket = calloc(1, sizeof(Bucket));
	if (!bucket) {
		fprintf(stderr, "Failed to allocate memory for HashMap bucket\n");
		return -1;
	}
	bucket->key = key;
	bucket->value = value;
	bucket->next = map->buckets[index];
	map->buckets[index] = bucket;
	map->count++;
	return 0;
}

void *hashmap_get(HashMap *map, long long int key) {
	if (!map || !map->buckets) {
		fprintf(stderr, "HashMap is not initialized\n");
		return NULL;
	}
	size_t index = sdbm((const char *)&key) % map->size;
	Bucket *bucket = map->buckets[index];
	while (bucket) {
		if (bucket->key == key) {
			return bucket->value;
		}
		bucket = bucket->next;
	}
	return NULL;
}

int hashmap_remove(HashMap *map, long long int key) {
	if (!map || !map->buckets) {
		fprintf(stderr, "HashMap is not initialized\n");
		return -1;
	}
	size_t index = sdbm((const char *)&key) % map->size;
	Bucket *bucket = map->buckets[index];
	Bucket *prev = NULL;
	while (bucket) {
		if (bucket->key == key) {
			if (prev) {
				prev->next = bucket->next;
			} else {
				map->buckets[index] = bucket->next;
			}
			free(bucket);
			map->count--;
			return 0;
		}
		prev = bucket;
		bucket = bucket->next;
	}
	return -1;
}

void *hashmap_replace(HashMap *map, long long int key, void *value) {
	if (!map || !map->buckets) {
		fprintf(stderr, "HashMap is not initialized\n");
		return NULL;
	}
	void *old_value = hashmap_get(map, key);
	size_t index = sdbm((const char *)&key) % map->size;
	Bucket *b = map->buckets[index];
	while (b->key != key) b = b->next;
	b->value = value;
	return old_value;

}

void hashmap_destroy(HashMap *map) {
	if (!map) return;
	for (size_t i = 0; i < map->size; i++) {
		Bucket *bucket = map->buckets[i];
		while (bucket) {
			Bucket *next = bucket->next;
			free(bucket);
			bucket = next;
		}
	}
	free(map->buckets);
	free(map);
}

HashMap *hashmap_resize(HashMap *map, int size) {
	HashMap *new = hashmap_create(size);
	for (size_t i = 0; i < map->size; i++)
	{
		Bucket *c = map->buckets[i];
		while (c)
		{
			Bucket *temp = c;
			hashmap_insert(new, c->key, c->value);
			c = c->next;
			free(temp);
		}
	}
	free(map);
}