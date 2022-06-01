/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definicion de funciones del manejador de memoria
*/

#include "mmu.h"
#include "i386.h"

#include "kassert.h"

static pd_entry_t* kpd = (pd_entry_t*)KERNEL_PAGE_DIR;
static pt_entry_t* kpt = (pt_entry_t*)KERNEL_PAGE_TABLE_0;

static const uint32_t identity_mapping_end = 0x003FFFFF;
/*static const uint32_t user_memory_pool_end = 0x02FFFFFF;*/

static paddr_t next_free_kernel_page = 0x100000;
static paddr_t next_free_user_page = 0x400000;

/**
 * kmemset asigna el valor c a un rango de memoria interpretado 
 * como un rango de bytes de largo n que comienza en s 
 * @param s es el puntero al comienzo del rango de memoria
 * @param c es el valor a asignar en cada byte de s[0..n-1]
 * @param n es el tamaño en bytes a asignar
 * @return devuelve el puntero al rango modificado (alias de s)
*/
static inline void* kmemset(void* s, int c, size_t n) {
  uint8_t* dst = (uint8_t*)s;
  for (size_t i = 0; i < n; i++) {
    dst[i] = c;
  }
  return dst;
}

/**
 * zero_page limpia el contenido de una página que comienza en addr
 * @param addr es la dirección del comienzo de la página a limpiar
*/
static inline void zero_page(paddr_t addr) {
  kmemset((void*)addr, 0x00, PAGE_SIZE);
}

/*void mmu_init(void) {}*/


/**
 * mmu_next_free_kernel_page devuelve la dirección de la próxima página de kernel disponible
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de kernel
 */
paddr_t mmu_next_free_kernel_page(void) {
  paddr_t res = next_free_kernel_page; //A: Le agrego a la ultima página 1KB
  next_free_kernel_page += PAGE_SIZE;
  return res;
}

/**
 * mmu_next_free_user_page devuelve la dirección de la próxima página de usuarix disponible
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de usuarix
 */
paddr_t mmu_next_free_user_page(void) {
  paddr_t res = next_free_user_page; //A: Le agrego a la ultima página 1KB
  next_free_user_page += PAGE_SIZE;
  return res;
}

/**unsigned 
 * mmu_init_kernel_dir inicializa las estructuras de paginación vinculadas al kernel y
 * realiza el identity mapping
 * @return devuelve la dirección de memoria de la página donde se encuentra el directorio 
 * de páginas usado por el kernel
 */
paddr_t mmu_init_kernel_dir(void) {
  zero_page((paddr_t) kpd); //A: Limpio kpd

  uint32_t attrs = 0x003; //A: Ignored := 0b0000, 0, Ign := 0, A := 0, PCD := 0, PWT := 0, U/S := 0, R/W := 1, 1
  pd_entry_t kpt_entry = { //A: Armo la primera entrada
    .attrs = attrs,
    .pt = (KERNEL_PAGE_TABLE_0 >> 12),
  };
  kpd[0] = kpt_entry;

  for (unsigned int i = 0; i < (identity_mapping_end / PAGE_SIZE) + 1; i++) { //A: Mappeo todas páginas del kernel //NOTA: La division da -1
    pt_entry_t kpt_pentry = {
      .attrs = attrs,
      .page = i,
    };
    kpt[i] = kpt_pentry; 
  }

  tlbflush();
  return (KERNEL_PAGE_DIR | attrs);
}

/**
 * mmu_map_page agrega las entradas necesarias a las estructuras de paginación de modo de que
 * la dirección virtual virt se traduzca en la dirección física phy con los atributos definidos en attrs
 * @param cr3 el contenido que se ha de cargar en un registro CR3 al realizar la traducción
 * @param virt la dirección virtual que se ha de traducir en phy
 * @param phy la dirección física que debe ser accedida (dirección de destino)
 * @param attrs los atributos a asignar en la entrada de la tabla de páginas
 */
void mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) {
  pd_entry_t *pd = (pd_entry_t*) CR3_TO_PAGE_DIR(cr3),
             *pde = &pd[VIRT_PAGE_DIR(virt)];
  bool was_missing = (pde->attrs & 1) == 0;
  pde->attrs = attrs; //A: Le asigno los atributos //NOTA: Hay atributos "extra" para la página en sí que la pd ignora
  if (was_missing) { //A: Not present
    pde->pt = mmu_next_free_kernel_page() >> 12; //A: Le apunto a una página nueva en memoria
    zero_page(pde->pt << 12); //A: La limpio
  }
  
  pt_entry_t *pt = (pt_entry_t*)(pde->pt << 12),
             *pte = &pt[VIRT_PAGE_TABLE(virt)];
  pte->attrs = attrs; //A: Le asigno los atributos
  pte->page = phy >> 12; //A: Le saco el offset a la física

  tlbflush();
}

/**
 * mmu_unmap_page elimina la entrada vinculada a la dirección virt en la tabla de páginas correspondiente
 * @param virt la dirección virtual que se ha de desvincular
 * @return la dirección física de la página desvinculada
 */
paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt) {
  pd_entry_t *pd = (pd_entry_t*) CR3_TO_PAGE_DIR(cr3), //A: Recupero la pde
             *pde = &pd[VIRT_PAGE_DIR(virt)];
  pt_entry_t *pt = (pt_entry_t*)(pde->pt << 12), //A: Recupero la pte a borrar
             *pte = &pt[VIRT_PAGE_TABLE(virt)];

  paddr_t phy = pte->page | (virt & 0xFFF); //A: Recupero la physical
  pte->attrs = 0x000; pte->page = 0x00000; //A: La borro

  //TODO: ¿Querría eliminar la tabla también si está vacía?

  tlbflush();
  return phy;
}

#define DST_VIRT_PAGE 0xA00000
#define SRC_VIRT_PAGE 0xB00000

/**
 * copy_page copia el contenido de la página física localizada en la dirección src_addr a la página física ubicada en dst_addr
 * @param dst_addr la dirección a cuya página queremos copiar el contenido
 * @param src_addr la dirección de la página cuyo contenido queremos copiar
 * 
 * Esta función mapea ambas páginas a las direcciones SRC_VIRT_PAGE y DST_VIRT_PAGE, respectivamente, realiza
 * la copia y luego desmapea las páginas. Usar la función rcr3 definida en i386.h para obtener el cr3 actual
 */
static inline void copy_page(paddr_t dst_addr, paddr_t src_addr) {
  uint32_t attrs = 0x003;
  mmu_map_page(rcr3(), DST_VIRT_PAGE, dst_addr, attrs); 
  mmu_map_page(rcr3(), SRC_VIRT_PAGE, src_addr, attrs);

  uint8_t *dst = (uint8_t*) DST_VIRT_PAGE,
           *src = (uint8_t*) SRC_VIRT_PAGE;
  for (size_t i = 0; i < PAGE_SIZE; i++)
    dst[i] = src[i];

  mmu_unmap_page(rcr3(), DST_VIRT_PAGE); 
  mmu_unmap_page(rcr3(), SRC_VIRT_PAGE);
}

void test_copy_page(void) {
  uint32_t src = 0x000000,
           dst = 0x400000;

  copy_page(dst, src);
}

 /**
 * mmu_init_task_dir inicializa las estructuras de paginación vinculadas a una tarea cuyo código se encuentra en la dirección phy_start
 * @pararm phy_start es la dirección donde comienzan las dos páginas de código de la tarea asociada a esta llamada
 * @return el contenido que se ha de cargar en un registro CR3 para la tarea asociada a esta llamada
 */
paddr_t mmu_init_task_dir(paddr_t phy_start) {
  vaddr_t v_start = 0x8000000;
  uint32_t attrs = 0x005; //A: Ignored := 0b0000, 0, Ign := 0, A := 0, PCD := 0, PWT := 0, U/S := 1, R/W := 0, 1
  
  //S: Le creo su propio cr3 con los atributos correctos
  paddr_t cr3 = (mmu_next_free_kernel_page() | attrs); 
  zero_page(CR3_TO_PAGE_DIR(cr3)); //A: La limpio

  //S: Le mapeo al kernel
  copy_page(cr3, rcr3()); 

  //S: Para el código
  mmu_map_page(cr3, v_start, phy_start, attrs);
  mmu_map_page(cr3, v_start + PAGE_SIZE, phy_start + PAGE_SIZE, attrs);

  //S: Para el stack
  uint32_t stack_attrs = 0x007; //A: Ignored := 0b0000, 0, Ign := 0, A := 0, PCD := 0, PWT := 0, U/S := 1, R/W := 1, 1
  mmu_map_page(cr3, v_start + 2 * PAGE_SIZE, mmu_next_free_user_page(), stack_attrs);

  return cr3;
}

