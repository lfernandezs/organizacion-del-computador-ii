1. Definir las estructuras de las tareas disponibles para ser ejecutadas.
2. Tener un **scheduler** que determine la tarea que le toca ejecutarse en un período de tiempo y el mecanismo de intercambio de tareas de la CPU.
3. Iniciar el kernel con una tarea inicial y tener una _tarea idle_ para cuando no haya tareas en ejecución.

# Ejercicios

1. .
Tarea: es una unidad de trabajo que el procesador puede despachar, ejecutar y suspender. Puede ser usada para ejecutar un programa.

En memoria, una tarea tiene:
* Espacio de ejecución: páginas mapeadas donde va a tener el código, datos y pilas.
* Segmento de Estado (TSS): una región de memoria que almacena el estado de una tarea, con un formato específico para que podamos iniciarla/reanudarla. La información que se guarda en esta región es:
	* Registros de propósito general.
	* Registros de segmento de la tarea y segmento de la pila de nivel 0.
	* Flags.
	* CR3
	* EIP

Scheduler: es un módulo de software que administra la ejecución de tareas/procesos. Cada vez que se pasa de una tarea a otra ocurre un cambio de contexto.

Para un sistema que tiene dos tareas debemos tener:
* Dos TSS con la información inicial de cada una de las tareas.
* Dos descriptores de segmento apuntando cada uno a su respectivo TSS.
* Dos selectores de segmento para apuntar a los descriptores.

**Descriptor de TSS**
Base:	Donde comienza el segmento.
Limit:	Último byte que pertenece al segmento.
AVL:	Available for use by system software.
B:	Ocupado.
DPL:	Descriptor privilege level.
G:	Granularidad.
P:	Segment present.
Type:	Tipo de segmento (TSS descriptor)

**Selector de TSS**
Selector: selector.
Base address, limit: ??

2. .
Cambio de contexto: el contexto de una tarea es el conjunto de valores de los registros de la CPU que se guardan en la TSS en el momento en que se está ejecutando.
Cuando hacemos un cambio de contexto, guardamos este conjunto de registros de una tarea, y restauramos el de otra. El procesador de encarga de ir copiando esta información en cada cambio de contexto.

Task Register: almacena el selector de segmento de la tarea en ejecución. Este selector de segmento nos indica en qué lugar de la GDT se encuentra el descriptor del TSS. Este descriptor nos indica donde se encuentra el TSS. El TSS almacena una foto del contexto de ejecucion de la tarea.
Entonces, si el registro TR puede acceder a el TSS, puede obtener toda la información para continuar la ejecución de esa tarea (EIP, registros, flags).

?: 10 - Visible part, invisible part? Por qué hay un base address y un segment limit en el selector de segmento del TSS, no debería estar eso en el descriptor?

3. .
Apenas iniciamos el kernel, hay que cargar la tarea inicial. Para hacerlo, usamor la instrucción LDTR que toma como parámetro el selector de la tarea en la GDT.
Luego, hay que saltar a la tarea idle. Hay que saltar al selector con un JMP y el valor que pongamos en el offset es ignorado. `JMP SELECTOR_TAREA_IDLE:0`.

Esto va a cambiar el valor del registro TR apuntando a la TSS de la tarea idle y producir el cambio de contexto.

4. .
El scheduler administra la  ejecución de las tareas. Utiliza una política para decidir cuál es la próxima tarea a ejecutar y lo hace en cada tic de reloj.

5. .
¿Cada tic de reloj es literal? O sea, puede que no complete una sola instrucción en un ciclo? Si es así, cómo sabe en qué parte de la instrucción tiene que continuar ya que el EIP cambia al inicio de cada instrucción?

6. .
a) `tss_dgt_entry_for_task` toma la tss de una tarea y devuelve su descriptor correspondiente.
b) 

11. .
```assembly
global _isr32
_isr32:
    pushad	; Pushea todos los registros de propósito 
            ; general.
    call pic_finish1 ; ?
    call sched_next_task ; Obtiene la siguiente tarea.
    str cx	; Store Task Register: guarda el selector de
    		; segmento del task register en cx.
    cmp ax, cx
    je .fin	; Si ax == cx, salta a .fin.
    mov word [sched_task_selector], ax	; El nuevo selector 
    									; de tarea se 
    									; encuentra ahora en 
    									; ax
    jmp far [sched_task_offset]	; Se salta al código de la 
    							; tarea
    .fin:
    popad
    iret
```