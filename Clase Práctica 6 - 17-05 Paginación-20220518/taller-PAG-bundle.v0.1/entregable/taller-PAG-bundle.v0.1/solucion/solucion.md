La memoria física se divide en kernel, área libre kernel y área libre tareas. La administración de las áreas libres de memoria se realizará a partir de una región de memoria específica para cada una. Podemos comprenderlas como un arreglo predefinido de páginas y dos contadores de páginas, uno para el kernel y otro para el usuario, que indican cuál será la próxima página a emplear de cada región.

Las páginas del área libre kernel serán utilizadas para datos del kernel: directorios de páginas, tablas de páginas y pilas de nivel cero. Las páginas del área libre tareas serán utilizadas para datos de las tareas, stack de las mismas y memoria compartida bajo demanda.

# Ejercicios

La memoria que el procesador direcciona en el bus es la **memoria física**. Es una secuencia de 8 bytes. Cada byte tiene una dirección única, llamada **dirección física**.

**Modelos de memoria**

Los programas no acceden directamente a la dirección física. Sino que acceden a la memoria usando uno de los tres modelos de memoria: flat, segmentado o real address.

* Flat: La memoria se le muestra a un programa como un único y continuo espacio direccionable. Este espacio se llama **linear address space**. Se accede mediante bytes, con las direcciones dentro del rango de 0 a 2³²-1. Cualquier dirección en el linear address space se llama **dirección lineal**.

* Segmented: La memoria se le presenta a un programa como un grupo de espacios direccionables independientes llamados segmentos. El código, la información y las pilas se separan en estos segmentos. Para acceder a un byte en un segmento, el programa accede a una **dirección lógica**. Esta consiste de un selector de segmento y un offset.

  Internamente, todos los segmentos que son definidos por el sistema son mapeados a una dirección linear del procesador. Para acceder a una dirección de memoria, el procesador hace una traducción de lógico a linear.

* Real-address: este es el modelo de memoria para el procesador Intel 8086.  Está para soportar la compatibilidad.

**Paging and virtual memory**

Con los modelos flat y segmented, la dirección linear es mapeada al procesador de manera directa o a través de paging.

Cuando se utiliza el mecanismo de paging del procesador, el espacio de direccionamiento lineal es dividido en páginas, que son mapeadas a la memoria virtual. Las páginas de la memoria virtual, son entonces mapeadas a la memoria física. 

**a)** ¿Cuántos niveles de privilegio podemos definir en las estructuras de paginación?

Tiene dos niveles de privilegio supervisor o kernel (0) y usuario (1). Esto se lo indica en el bit U/S de la PTE (Page Table Entity).

**b)** ¿Cómo se traduce una dirección lógica en una dirección física? ¿Cómo participan el selector de segmento, el registro de control CR3, el directorio y la tabla de páginas?

selector de segmento??

En la CR3 podemos obtener la dirección del directorio de páginas. 

Obtenemos la dirección lineal y la dividimos en tres partes. (0-9) indican en qué directorio vamos a buscar la ubicación de la tabla de páginas (pd_index). (10-9) indican qué entrada de la tabla tiene la ubicación de la dirección base de la página (pt_index). Y los restantes 12 bits indican el desplazamiento desde la base de la página hacia donde se encuentra el dato que buscamos (page_offset).

**c)** ¿Cuál es el efecto de los siguientes atributos en las entradas de la tabla de página?

* D: Es seteado por la unidad de memoria del procesador cuando se escribe en la página.
* A: Es seteado por la unidad de memoria del procesador cuando se escribe o se lee en la página.
* PCD: hace que la página no se almacene en la memoria rápida.
* PWT: hace que al escribir la escritura se refleje en la cache y memoria a la vez.
* U/S: indica si la página puede ser accedida por el usuario o solo por el kernel.
* R/W: indica si la página puede leerse y escribirse.
* P: indica si la página se encuentra cargada en memoria o no.

**d)** Suponiendo que el código de la tarea ocupa dos páginas y utilizaremos una página para la pila de la tarea. ¿Cuántas páginas hace falta pedir a la unidad de manejo de memoria para el directorio, tablas de página y la memoria de una tarea?

El directorio de páginas ocupa una página entera. Luego, necesitaremos otra página para almacenar la tabla de página correspondiente a las que vayamos a utilizar. Una página más para la pila y otras dos para el código de la tarea. Entonces tenemos 5 páginas.