# tp-2021-1c-DuroDeAprobar

 
 <p align="center">
  <img src="https://user-images.githubusercontent.com/62316777/125995719-436fd46a-9d69-4689-8611-d01ee7fc1bee.png" alt="Duro de Aprobar"/>
</p>


---

## Guía de deploy

Para instalar las dependencias y compilar el proyecto, ejecutar `install.sh` con el parámetro `-f`:

```
git clone https://github.com/sisoputnfrba/tp-2021-1c-DuroDeAprobar.git
cd tp-2021-1c-DuroDeAprobar
git checkout deploy2
./install.sh -f
```

Si solo se desea instalar las dependencias, ejecutar `install.sh -d`

Si solo se desea compilar los modulos, ejecutar `install.sh -c`

## Iniciando módulos

Los módulos podrán iniciarse ejecutando `mongo.sh`, `ram.sh` y `discordiador.sh`, en ese orden, con el parámetro correspondiente:

### Solo compilar:
```
./<modulo> -c
```

### Correr modulo con valgrind:
```
./<modulo> -v
```

### Iniciar modulo normalmente:
```
./<modulo> -i
```

