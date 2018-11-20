#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <list>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <time.h>

#include "of.h"
#include "ofOffPointsReader.h"
#include "Handler.hpp" 
#include "GL_Interactor.h"
#include "ColorRGBA.hpp"
#include "Cores.h"
#include "Point.hpp"
#include "printof.hpp"

#include "CommandComponent.hpp"
#include "MyCommands.hpp"

#include "ofVertexStarIteratorSurfaceVertex.h"


clock_t start_insert;
clock_t end_insert;
clock_t start_print;
clock_t end_print;



using namespace std;
using namespace of;

//Define o tamanho da tela.
scrInteractor *Interactor = new scrInteractor(900, 800);



// malha inserida em GL_Interactor.h

int type = 3;

int inicial = 167;

double determinanteMatriz(double mat[3][3]) 
{ 
    double ans; 
    ans = mat[0][0] * (mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2]) 
          - mat[0][1] * (mat[1][0] * mat[2][2] - mat[1][2] * mat[2][0]) 
          + mat[0][2] * (mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]); 
    return ans; 
} 
  
// resolve o sistema linear das coordenadas baricentricas utilizando a regra de cramer
void sistemaEquacaoCramer(double mat[3][4], double * cbar1, double * cbar2, double * cbar3) 
{
    double d[3][3] = { 
        { mat[0][0], mat[0][1], mat[0][2] }, 
        { mat[1][0], mat[1][1], mat[1][2] }, 
        { mat[2][0], mat[2][1], mat[2][2] }, 
    }; 
    double d1[3][3] = { 
        { mat[0][3], mat[0][1], mat[0][2] }, 
        { mat[1][3], mat[1][1], mat[1][2] }, 
        { mat[2][3], mat[2][1], mat[2][2] }, 
    }; 
    double d2[3][3] = { 
        { mat[0][0], mat[0][3], mat[0][2] }, 
        { mat[1][0], mat[1][3], mat[1][2] }, 
        { mat[2][0], mat[2][3], mat[2][2] }, 
    }; 
    double d3[3][3] = { 
        { mat[0][0], mat[0][1], mat[0][3] }, 
        { mat[1][0], mat[1][1], mat[1][3] }, 
        { mat[2][0], mat[2][1], mat[2][3] }, 
    }; 
  
    double D = determinanteMatriz(d); 
    double D1 = determinanteMatriz(d1); 
    double D2 = determinanteMatriz(d2); 
    double D3 = determinanteMatriz(d3); 
  
    // Caso 1
    if (D != 0) { 
        // Solucao unica
        double x = D1 / D; 
        double y = D2 / D; 
        double z = D3 / D; 
        *cbar1 = x;
        *cbar2 = y;
        *cbar3 = z;
    } 
} 
  


void coordenadasBaricentricas(){

    double xp, yp; //coordenadas do ponto P
    xp = Interactor->getPX();
    yp = Interactor->getPY();
    
    int id = inicial;
    
    // trata ponto inicial para que nao crie um caminho inicial aleatorio
    if (xp != 0.0 && yp != 0.0)
    {
       double cbar1, cbar2, cbar3;
       cbar1 = cbar2 = cbar3 = -1; 
       double xa, ya, xb, yb, xc, yc; 

       while (cbar1 <= 0 || cbar2 <= 0 || cbar3 <= 0)
       {
          Print->Face(malha->getCell(id), llblue);

          xa = malha->getVertex(malha->getCell(id)->getVertexId(0))->getCoord(0);
          ya = malha->getVertex(malha->getCell(id)->getVertexId(0))->getCoord(1);
          xb = malha->getVertex(malha->getCell(id)->getVertexId(1))->getCoord(0);
          yb = malha->getVertex(malha->getCell(id)->getVertexId(1))->getCoord(1);
          xc = malha->getVertex(malha->getCell(id)->getVertexId(2))->getCoord(0);
          yc = malha->getVertex(malha->getCell(id)->getVertexId(2))->getCoord(1);

          //monta matriz para resolver o sistema utilizando cramer

         double mat[3][4] = { 
            { xa, xb, xc, xp }, 
            { ya, yb, yc, yp }, 
            { 1, 1, 1, 1 }, 
        }; 

         sistemaEquacaoCramer(mat, &cbar1, &cbar2, &cbar3);

          int prox, aresta;          

          if (cbar1 > 0 && cbar2 > 0 && cbar3 > 0)
          {
             break;
          }
          else
          {
              if (cbar1 < cbar2 && cbar1 < cbar3)
              {
                 aresta = 0;
                 prox = malha->getCell(id)->getMateId(0);
              }
              if (cbar2 < cbar1 && cbar2 < cbar3)
              {
                 aresta = 1;
                 prox = malha->getCell(id)->getMateId(1);
              }
              if (cbar3 < cbar1 && cbar3 < cbar2)
              {
                 aresta = 2;
                 prox = malha->getCell(id)->getMateId(2);
              }
              
              if (prox == -1)
              {
                 if (aresta == 0) //BC
                    Print->Edge(malha->getVertex(malha->getCell(id)->getVertexId(1)), malha->getVertex(malha->getCell(id)->getVertexId(2)), blue, 3.0);
                 if (aresta == 1) //AC
                    Print->Edge(malha->getVertex(malha->getCell(id)->getVertexId(0)), malha->getVertex(malha->getCell(id)->getVertexId(2)), blue, 3.0);
                 if (aresta == 2) //AB
                    Print->Edge(malha->getVertex(malha->getCell(id)->getVertexId(0)), malha->getVertex(malha->getCell(id)->getVertexId(1)), blue, 3.0);
                 break;
              }
          }
          id = prox;
       }
    }
}

void RenderScene(void){
	allCommands->Execute();
	
    Print->Face(malha->getCell(inicial), dblue);
    coordenadasBaricentricas();

	Print->FacesWireframe(malha,grey,3);
	glFinish();
	glutSwapBuffers();
}

void HandleKeyboard(unsigned char key, int x, int y){	
	
	
	
	double coords[3];
	char *xs[10];
	allCommands->Keyboard(key);
	
	switch (key) {

		case 'e':
			exit(1);
		break;
		case 'v':
            //um numero aleatorio entre 1 e 410 para ser a nova celula inicial
			inicial = rand() % 410 + 1;
		break;
	

	}
    
	Interactor->Refresh_List();
	glutPostRedisplay();

}

using namespace std;

int main(int argc, char *argv[])
{
  ofRuppert2D<MyofDefault2D> ruppert;
  ofPoints2DReader<MyofDefault2D> reader;
  ofVtkWriter<MyofDefault2D> writer;
  Interactor->setDraw(RenderScene);
	meshHandler.Set(new TMesh());

  
    reader.readOffFile("Brasil.off");
    ruppert.execute2D(reader.getLv(),reader.getLids(),true);
  
  meshHandler = ruppert.getMesh();
  malha = ruppert.getMesh();
  
  
  Print = new TPrintOf(meshHandler);

	allCommands = new TMyCommands(Print, Interactor);

	double a,x1,x2,y1,y2,z1,z2; 

	of::ofVerticesIterator<TTraits> iv(&meshHandler);

	iv.initialize();
	x1 = x2 = iv->getCoord(0);
	y1 = y2 = iv->getCoord(1);
	z1 = z2 = iv->getCoord(2);
	
	
    //-iv->getCoord(1) para deixar o mapa certo
	for(iv.initialize(); iv.notFinish(); ++iv){
        iv->setCoord(1, -iv->getCoord(1)); 
		if(iv->getCoord(0) < x1) x1 = a = iv->getCoord(0);
		if(iv->getCoord(0) > x2) x2 = a = iv->getCoord(0);
		if(iv->getCoord(1) < y1) y1 = a = iv->getCoord(1);
		if(iv->getCoord(1) > y2) y2 = a = iv->getCoord(1);
		if(iv->getCoord(2) < z1) z1 = a = iv->getCoord(2);
		if(iv->getCoord(2) > z2) z2 = a = iv->getCoord(2);
	}

	double maxdim;
	maxdim = fabs(x2 - x1);
	if(maxdim < fabs(y2 - y1)) maxdim = fabs(y2 - y1);
	if(maxdim < fabs(z2 - z1)) maxdim = fabs(z2 - z1);

	maxdim *= 0.3; 
	Point center((x1+x2)/2.0, (y1+y2)/2.0, (y1+y2)/2.0 );
	Interactor->Init(center[0]-maxdim, center[0]+maxdim,
					center[1]-maxdim, center[1]+maxdim,
					center[2]-maxdim, center[2]+maxdim);

	AddKeyboard(HandleKeyboard);

	allCommands->Help(std::cout);
	std::cout<< std::endl<< "Press \"?\" key for help"<<std::endl<<std::endl;
	double t;
	
	Init_Interactor();

  
  return EXIT_SUCCESS;
}

