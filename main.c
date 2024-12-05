#include <stdio.h>
#include <stdlib.h>

#include "vm.h"


int main()
{
    Vm vm;
    init_vm(&vm);

    pushArray(&vm, 12);
    pushArray(&vm, 12);
    pop(&vm);
    pop(&vm);

    newObjString(&vm, "hola mundo", 10);
    newObjString(&vm, "hola mundo", 10);

    gc(&vm);

    return 0;
}
