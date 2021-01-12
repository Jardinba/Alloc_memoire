/* On inclut l'interface publique */
#include "mem.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* Définition de l'alignement recherché
 * Avec gcc, on peut utiliser __BIGGEST_ALIGNMENT__
 * sinon, on utilise 16 qui conviendra aux plateformes qu'on cible
 */
#ifdef __BIGGEST_ALIGNMENT__
#define ALIGNMENT __BIGGEST_ALIGNMENT__
#else
#define ALIGNMENT 16
#endif

/* structure placée au début de la zone de l'allocateur

   Elle contient toutes les variables globales nécessaires au
   fonctionnement de l'allocateur

   Elle peut bien évidemment être complétée
*/
struct allocator_header
{
	size_t memory_size;
	mem_fit_function_t *fit;
	struct fb *first;
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header)
 */
static void *memory_addr;

static inline void *get_system_memory_addr()
{
	return memory_addr;
}

static inline struct allocator_header *get_header()
{
	struct allocator_header *h;
	h = get_system_memory_addr();
	return h;
}

static inline size_t get_system_memory_size()
{
	return get_header()->memory_size;
}

struct fb
{
	size_t size;
	struct fb *next;
};

/* fonction qui est apeler au début, tout est à faire */
void mem_init(void *mem, size_t taille)
{
	memory_addr = mem;
	*(size_t *)memory_addr = taille;
	/* On vérifie qu'on a bien enregistré les infos et qu'on
	 * sera capable de les récupérer par la suite
	 */
	assert(mem == get_system_memory_addr());
	assert(taille == get_system_memory_size());

	//h est un pointeur sur l'allocateur memoire
	struct allocator_header *h;
	h = (struct allocator_header *)mem; //on définit h
	h->memory_size = taille;			//on remplit la structure
	h->first = (struct fb *)(mem + sizeof(struct allocator_header));
	h->first = (struct fb *)(h + 1);

	struct fb *zonelibre = h->first;
	zonelibre->size = taille - sizeof(struct allocator_header);
	zonelibre->next = NULL;

	h->fit = &mem_fit_first;
	mem_fit(&mem_fit_first);
}

void mem_show(void (*print)(void *, size_t, int))
{
	void *cur_bloc = get_system_memory_addr() + sizeof(struct allocator_header);
	struct fb *next_freebloc = get_header()->first;
	while (cur_bloc < get_system_memory_addr() + get_system_memory_size())
	{
		int est_bloc_libre;
		if (cur_bloc == (struct bb *)next_freebloc)
		{
			est_bloc_libre = 1;
			next_freebloc = next_freebloc->next;
		}
		else
		{
			est_bloc_libre = 0;
		}
		size_t size = *(size_t*)cur_bloc;
		print(cur_bloc, size, est_bloc_libre);
		cur_bloc = (void *)cur_bloc + size;
	}
}

void mem_fit(mem_fit_function_t *f)
{
	get_header()->fit = f;
}

void *mem_alloc(size_t taille) {

    if ((sizeof(size_t) + taille) < sizeof(struct fb)) {
		taille = sizeof(struct fb) - sizeof(size_t);
	}
    if (taille % ALIGNMENT != 0) {
		taille += (ALIGNMENT - taille % ALIGNMENT);
	}
    size_t sizeFull = taille + sizeof(size_t);
    
    __attribute__((unused)) /* juste pour que gcc compile ce squelette avec -Werror */
    struct fb *fb = get_header()->fit(get_header()->first, sizeFull);
    if (fb == NULL) {
		return NULL;
	}
    struct fb *cur_block = get_header()->first;
    while (cur_block != NULL && cur_block->next < fb) {
        cur_block = cur_block->next;
	}
	if (cur_block > fb) {
		cur_block = NULL;
	}
    if (cur_block != NULL && sizeFull == fb->size) {
        cur_block->next = fb->next;
        if (fb == get_header()->first)
    		get_header()->first = fb->next;
    } else {
        struct fb *suivant = (void*)fb + sizeFull;
        *suivant = (struct fb){
            fb->size - sizeFull,
            fb->next
        };
        fb->size = sizeFull;
        if (cur_block != NULL) {
			cur_block->next = suivant;
		}
	    if (fb == get_header()->first) {
			get_header()->first = suivant;
		}
    }
    return (void*)fb + sizeof(size_t);
}

void mem_free(void* mem) {
	struct fb *cur_block = (struct fb*)(mem - sizeof(size_t));
	struct fb *bf = NULL;
	struct fb *nx = get_header()->first;
    while (nx != NULL && nx < cur_block) {
        bf = nx; nx = nx->next;
    }
    *cur_block = (struct fb) {
        mem_get_size(mem),
        nx
    };
	int bff = 0, nxf = 0;
	if (bf != NULL && cur_block == (void*)bf + bf->size) {
		bff = 1;
	}
	if (nx != NULL && nx == (void*)cur_block + cur_block->size) {
		nxf = 1;
	}
    if (nxf == 1) {
        cur_block->size += nx->size;
        cur_block->next = nx->next;
    }
    if (bff == 1) {
        bf->size += cur_block->size;
        bf->next = cur_block->next;
    } else if (bf != NULL) {
        bf->next = cur_block;
	} else {
		get_header()->first = cur_block;	
	}
}

struct fb *mem_fit_first(struct fb *list, size_t size) {
	while (list != NULL) {
		if (list->size >= size + 8) {
			return list;
		}
		list = list->next;
	}
	
	return NULL;
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone)
{
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
	return *(size_t*) (zone - sizeof(size_t));
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb *mem_fit_best(struct fb *list, size_t size)
{
	return NULL;
}

struct fb *mem_fit_worst(struct fb *list, size_t size)
{
	return NULL;
}
