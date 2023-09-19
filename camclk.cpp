using namespace std;


/* sample to emule kbhit()

#include <stdio.h>
#include <ncurses.h>

int main(int argc, char *argv)
{
    char input;

    initscr(); // entering ncurses mode
    raw();     // CTRL-C and others do not generate signals
    noecho();  // pressed symbols wont be printed to screen
    cbreak();  // disable line buffering
    while (1) {
        erase();
        mvprintw(1,0, "Enter symbol, please");
        input = getch();
        mvprintw(2,0, "You have entered %c", input);
        getch(); // press any key to continue
    }
    endwin(); // leaving ncurses mode
    return 0;
}

*/

//Entorno

#include <stdlib.h> // para random()

//#include <conio2.h> // manejo de pantalla
#include <unistd.h>	//  POSIX flags
#include <time.h>	//  clock_gettime(), time()
#include <sys/time.h> //	gethrtime(), gettimeofday()
#include <ncurses.h>

//#include <iostream>
#include <sys/select.h> //para definir kbhit


//Objetos:
#define AVION "|_>"
#define RESCATE "+_+"
#define DESGRACIADO "*"
#define TEXTAYUDA "[ESC] Termina | 1=Izq | 0=Der"

#define BORRA_AVION   "   "
#define BORRA_RESCATE "   "
#define BORRA_DESGRACIADO " "

//Coordenadas de referencia
#define ALTURA_AVION 1 //Fila 1
#define SUELO 22 //Fila 23
#define XRESCATE_INICIO 40

#define MARCADOR 23 //Fila de ayuda y marcador
#define COL_RECUPERADOS 14 //Columna del n£mero de recuperados
#define COL_PERDIDOS    35 //Columna del n£mero de perdidos
#define COL_AYUDA       50

//LÍMITE DEL JUEGO
#define NUMERO_PASAJEROS 300

//MACROFUNCIONES


#define gotoxy(x,y) move(y,x)
#define clrscr() erase()
//#define kbhit() getch()
#define xprintf(arg) printw(arg)


#define borra_avion(x_avion,y) gotoxy(x_avion,y);xprintf(BORRA_AVION);

#define dibuja_avion(x_avion,y) gotoxy(x_avion,y); xprintf(AVION);

#define borra_pasajero(x,y) gotoxy(x,y);xprintf(" ");
#define dibuja_pasajero(x,y) gotoxy(x,y);xprintf(DESGRACIADO);

//NEW clock()
#define RETARDO_1 0.1/2
#define RETARDO_2 0.05

//PROTOTIPOS DE FUNCIONES
//double GetTime();
//#define getRealTime() GetTime()
double GetRealTime(); //función

int xrandom(int max);
void score(void);
void Delay(double seconds);
void MueveCamilleros();
#define retardo(seconds) Delay(seconds);

//Variables globales
int recuperados,perdidos;
int x_rescate=XRESCATE_INICIO;

int xdesgraciado; //columna del pasajero callendo
int ydesgraciado; //fila del pasajero callendo
int x_avion;
int n_pasajeros; //cargo total de pasajeros.
int lanzamiento; //booleana para indicar si hay un pasajero callendo o no

double startTime, endTime;

int c=0; //tecla pulsada por el usuario
int fin=0; // booleana para finalizar, valdrá 1 cuando se pulse ESCAPE
//FUNCION PRINCIPAL



int kbhit(void)
{
struct timeval tv;
fd_set read_fd;

/* Do not wait at all, not even a microsecond */
tv.tv_sec=0;
tv.tv_usec=0;

/* Must be done first to initialize read_fd */
FD_ZERO(&read_fd);

/* Makes select() ask if input is ready:
* 0 is the file descriptor for stdin */
FD_SET(0,&read_fd);

/* The first parameter is the number of the
* largest file descriptor to check + 1. */
if(select(1, &read_fd,NULL, /*No writes*/NULL, /*No exceptions*/&tv) == -1)
return 0; /* An error occured */

/* read_fd now holds a bit map of files that are
* readable. We test the entry for the standard
* input (file 0). */

if(FD_ISSET(0,&read_fd))
/* Character pending on stdin */
return 1;

/* no characters were pending */
return 0;
}

int main(void)
{


//Entrada de teclado

//INICIALIZACIàN DE VARIABLES
n_pasajeros=NUMERO_PASAJEROS; //cargo total de pasajeros


//Marcador del juego
recuperados=0,perdidos=0;

//Posici¢n de los camilleros (donde est  el camillero de la izquierda)
x_rescate=XRESCATE_INICIO; //posici¢n inicial de los camilleros

//booleana para indicar si hay un pasajero callendo o no
lanzamiento=0;

//Semilla para la funci¢n random():
srand(clock());


//NCURSES:
    initscr(); // entering ncurses mode
    raw();     // CTRL-C and others do not generate signals
    noecho();  // pressed symbols wont be printed to screen
    cbreak();  // disable line buffering
    curs_set(0); //hide the cursor
//COMIENZA LA ACCIàN  úúúúúúúúúÀÄ>
clrscr();

//Apagar el cursor
//_setcursortype(_NOCURSOR);

gotoxy(1,MARCADOR);

score();

//xprintf("Recuperados=%3d Perdidos=%3d | Teclas: 1=Izq. 0=Der. [ESC]=SALIR | Pasaje=%3d",recuperados,perdidos,n_pasajeros);

//Columna inicial del avi¢n==1
x_avion=1;

//La columna del pasajero ser  entre 2 y 78, ya que de lo contrario no
// lo podr¡an recoger los camilleros +-+, la camilla est  en el rango
// 2-79
xdesgraciado=xrandom(77)+2; //columna de la que ser  arrojado el pasajero
//refresh();
do
{
//Dibujar avión:
   dibuja_avion(x_avion,ALTURA_AVION);
   refresh();
   if(x_avion==78) //Si el avi¢n llega al final de la pantalla
   {
      borra_avion(x_avion,ALTURA_AVION);
      x_avion=1; //Restablece columna de avi¢n para siguiente pasada
   }

//Si el pasajero est  en las coordenadas del avi¢n:
   if(x_avion==xdesgraciado||x_avion==xdesgraciado-1||x_avion==xdesgraciado+1)
   {
      dibuja_pasajero(xdesgraciado,ydesgraciado);
      //Bandera para indicar que hay un pasajero callendo
      lanzamiento=1;
   }
   if(lanzamiento)
   {
      //Borra pasajero, avanza una columna y vuelve a pintar pasajero
      borra_pasajero(xdesgraciado,ydesgraciado);
      ydesgraciado++; //Avanza una columna
      dibuja_pasajero(xdesgraciado,ydesgraciado);

      retardo(RETARDO_1); //cuando el pasajero est  cayendo, hay un retardo adicional

      //Cuando el pasajero llega al suelo ...
      if(ydesgraciado==SUELO)
      {
      //JARM
        lanzamiento=0;

        //Borra al pasajero
        borra_pasajero(xdesgraciado,ydesgraciado);

        score(); //Comprueba si el rescate a ido bi‚n y actualiza marcador
        //prepara la col. del siguiente pasajero
        xdesgraciado=xrandom(77)+2; //2-79

        //Pasajero en la puerta del avi¢n:
        ydesgraciado=1;
      }

   }//FIN if(lanzamiento)

   retardo(RETARDO_1);

//Dibujar camilleros
   gotoxy(x_rescate,SUELO);
   xprintf(RESCATE);

//Esperar tecla
//El while fuerza que los camilleros se muevan + r pido si tecla pulsada
   while(kbhit())
   {
       MueveCamilleros();
   } //FIN while(kbhit())


   retardo(RETARDO_1);

   //Borrar avi¢n -> al cominezo del bucle lo redibuja
   borra_avion(x_avion,ALTURA_AVION);

   //avanzar una posici¢n el avi¢n
   x_avion++; //

refresh();
   //Al principio del bucle do while() se pinta el avi¢n
}while(!fin||n_pasajeros <= 0);

//Restaurar cursor:
//_setcursortype(1);
clrscr();

//fin ncurses __^
curs_set(1);
endwin();

return 0;
}
//FIN MAIN()

//NUMEROS ALEATORIOS ENTRE 0 Y max
int xrandom(int max)
{
int n;
   //n=(int) random()%max;
   n=rand()%max;
   return n;
}
//MARCADOR
void score(void)
{
//#define xprintf(arg) printw(arg)
   if(xdesgraciado==x_rescate+1)
   {
      recuperados+=1;
   }
   else
      perdidos+=1;

n_pasajeros--;
gotoxy(1,MARCADOR); //(this)
printw("Recuperados=%3d Perdidos=%3d | Teclas: 1=Izq. 0=Der. [ESC]=SALIR | Pasaje=%3d",recuperados,perdidos,n_pasajeros);
//printw("Recuperados=");

}


/**
 * Returns the real time, in seconds, or -1.0 if an error occurred.
 *
 * Time is measured since an arbitrary and OS-dependent start time.
 * The returned real time is only useful for computing an elapsed time
 * between two calls to this function.
 */
double GetRealTime( )
{
        struct timespec ts;
		const clockid_t id = CLOCK_MONOTONIC_RAW;

		if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 )
			return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
        else
            return -1;


}


void Delay(double seconds){
startTime=GetRealTime();
endTime=GetRealTime();
do{
    endTime=GetRealTime();
    if(kbhit()){MueveCamilleros();}

}while((endTime-startTime)<seconds);
}

void MueveCamilleros(){
c=getch(); //tecla pulsada por el usuario
      //xprintf("%d",c);
      switch(c)
      {
         case 27:
            fin=1;
            break;
         case 49: // caracter '1' => mover a la izquierda
            //Borrar rescate ...

            gotoxy(x_rescate,SUELO);
            xprintf(BORRA_RESCATE);

//!!!Perfilaci¢n si x_rescate==0 no hay que reubicar
            --x_rescate;
            if(x_rescate==0) x_rescate=1;

            //...y reubicar rescate
            gotoxy(x_rescate,SUELO);
            xprintf(RESCATE);

            break;
         case 48: //caracter '0' => mover a la derecha
//!!!Perfilaci¢n: el proceso Borrar rescate y reubicar se repite dos veces
//!!! => bandera_izq o bandera_der y hacer proceso fuera de switch
         //Borrar rescate ...
            gotoxy(x_rescate,SUELO);
            xprintf(BORRA_RESCATE);

            //++x_rescate;
            if(x_rescate<=77) ++x_rescate;

            //...y reubicar rescate
            gotoxy(x_rescate,SUELO);
            xprintf(RESCATE);

            break;
      }
}
