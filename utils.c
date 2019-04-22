#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define MAX(x, y) ((x) >= (y) ? (x) : (y))

#define buf_hdr(b) ((Buf *)((char *)(b) - offsetof(Buf, buf)))
#define buf_len(b) ((b) ? buf_hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf_hdr(b)->cap : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : ((b) = buf_grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit(b, 1 + buf_len(b)), (b)[buf_hdr(b)->len++] = (__VA_ARGS__))

typedef struct Buf
{
	size_t len;
	size_t cap;
	char buf[];
} Buf;

void *buf_grow(const void *buf, size_t size, size_t elem_size)
{
	size_t new_cap = MAX(16, MAX(1 + 2 * buf_cap(buf), size));
	assert(size <= new_cap);
	size_t new_size = offsetof(Buf, buf) + new_cap * elem_size;
	Buf *new_hdr = NULL;
	if (buf)
	{
		new_hdr = realloc(buf_hdr(buf), new_size);
	}
	else
	{
		new_hdr = malloc(new_size);
		new_hdr->len = 0;
	}
	new_hdr->cap = new_cap;
	return new_hdr->buf;
}

int buf_pop_int(const void *buf)
{
	assert(buf_len(buf) - 1);
	int *int_buf = (int *)buf;
	int val = int_buf[--buf_hdr(buf)->len];
	printf("%d", val);
	return val;
}

char buf_pop_char(const void *buf)
{
	assert(buf_len(buf) - 1);
	char *char_buf = (char *)buf;
	char val = char_buf[--buf_hdr(buf)->len];
	printf("%c", val);
	return val;
}

void fatal(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("fatal: ");
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
	exit(1);
}

typedef struct Arena
{
	char *ptr;
	char *end;
	char **blocks;
} Arena;

Arena arena;

#define ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ALIGN_UP(n, a) ALIGN_DOWN((n) + (a) - 1, (a))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((uintptr_t)(p), (a)))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((uintptr_t)(p), (a)))

#define ALIGN 8
#define BLOCK_SIZE 1024

void arena_grow(Arena *arena, size_t size)
{
	size_t new_size = ALIGN_UP(MAX(BLOCK_SIZE, size), ALIGN);
	arena->ptr = malloc(new_size);
	arena->end = arena->ptr + new_size;
	buf_push(arena->blocks, arena->ptr);
}

char *arena_alloc(Arena *arena, size_t size)
{
	if (size > (size_t)(arena->end - arena->ptr))
	{
		arena_grow(arena, size);
	}
	char *ptr = arena->ptr;
	arena->ptr = ALIGN_UP_PTR(arena->ptr + size, ALIGN);
	return ptr;
}

typedef struct HashString
{
	uint64_t hash;
	const char *string;
} HashString;

HashString *hash_strings;

uint64_t hash_string(const char *string, size_t length)
{
	static const uint64_t fnv_prime = 0x100000001b3ull;
	static const uint64_t fnv_initializer = 0xcbf29ce484222325ull;
	uint64_t hash = fnv_initializer;
	for (size_t i = 0; i < length; i++)
	{
		hash ^= string[i];
		hash *= fnv_prime;
	}
	return hash;
}

const char *str_intern(const char *string, size_t length)
{
	uint64_t hash = hash_string(string, length);
	for (size_t i = 0; i < buf_len(hash_strings); i++)
	{
		if (hash_strings[i].hash == hash && strncmp(hash_strings[i].string, string, length) == 0)
		{
			return hash_strings[i].string;
		}
	}
	char *new_string = arena_alloc(&arena, length);
	memcpy(new_string, string, length);
	new_string[length] = 0;
	buf_push(hash_strings, (HashString) { hash, new_string });
	return new_string;
}
