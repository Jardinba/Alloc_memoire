/* On inclut l'interface publique */
#include "mem.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

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
struct allocator_header {
        size_t memory_size;
	mem_fit_function_t *fit;
	struct fb* first;
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header)
 */
static void* memory_addr;

static inline void *get_system_memory_addr() {
	return memory_addr;
}

static inline struct allocator_header *get_header() {
	struct allocator_header *h;
	h = get_system_memory_addr();
	return h;
}

static inline size_t get_system_memory_size() {
	return get_header()->memory_size;
}


struct fb {
	size_t size;
	struct fb* next;
};

struct bb {
	size_t size;
};

/* fonction qui est apeler au début, tout est à faire */
void mem_init(void* mem, size_t taille)
{
        memory_addr = mem;
        *(size_t*)memory_addr = taille;
	/* On vérifie qu'on a bien enregistré les infos et qu'on
	 * sera capable de les récupérer par la suite
	 */
	assert(mem == get_system_memory_addr());
	assert(taille == get_system_memory_size());

	//h est un pointeur sur l'allocateur memoire
	struct allocator_header* h;
	h = (struct allocator_header*)mem; //on définit h
	h->memory_size = taille; //on remplit la structure
	h->first = (struct fb*)(mem + sizeof(struct allocator_header));
	h->first = (struct fb*)(h + 1);

	struct fb* zonelibre = h->first;
	zonelibre->size = taille - sizeof(struct allocator_header);
	zonelibre->next = NULL;
	
	h->fit = &mem_fit_first;
	mem_fit(&mem_fit_first);
}

void mem_show(void (*print)(void *, size_t, int)) {
	struct bb *cur_bloc;
	struct fb *next_freebloc = get_header()->first;
	cur_bloc = (struct bb*)(get_header()+1);
	struct bb *fin_memoire = get_system_memory_addr() + get_header()->memory_size;
	while (cur_bloc < fin_memoire) {
		int est_bloc_libre;
		if (cur_bloc == (struct bb*)next_freebloc) {
			est_bloc_libre = 1;
			next_freebloc = next_freebloc->next;
		} else {
			est_bloc_libre = 0;
		}
		print(cur_bloc, cur_bloc->size, est_bloc_libre);
		cur_bloc = (void*)cur_bloc + cur_bloc->size;
	}
}

void mem_fit(mem_fit_function_t *f) {
	get_header()->fit = f;
}

void *mem_alloc(size_t taille) {
	/* ... */
	__attribute__((unused)) /* juste pour que gcc compile ce squelette avec -Werror */
	struct fb *fb=get_header()->fit(/*...*/NULL, /*...*/0);
	/* ... */
	return NULL;
}


void mem_free(void* mem) {
}


struct fb* mem_fit_first(struct fb *list, size_t size) {
	return NULL;
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone) {
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
	return 0;
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb* mem_fit_best(struct fb *list, size_t size) {
	return NULL;
}

struct fb* mem_fit_worst(struct fb *list, size_t size) {
	return NULL;
}

