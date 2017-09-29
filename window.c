/*!\file window.c
 *
 * \brief Walking on finite plane with skydome textured with a
 * triangle-edge midpoint-displacement algorithm.
 *
 * \author Farès BELHADJ, amsi@ai.univ-paris8.fr
 * \date February 9 2017
 * \edition Vincent Le Jeune
 * \February 21 2017 
 * \last edited : 02 May 2017
 */
#include <math.h>
#include <GL4D/gl4dg.h>
#include <GL4D/gl4duw_SDL2.h>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <time.h>
#include <GL/glut.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

extern void assimpInit(const char * filename);
extern void assimpDrawScene();
extern void assimpQuit(void);

static void quit(void);
static void initGL(void);
static void initData(void);
static void resize(int w, int h);
static void idle(void);
static void keydown(int keycode);
static void keydown2(int keycode);
static void keyup(int keycode);
static void draw(void);
static void mouse(int button, int state, int x, int y);

static void loosepdv (float vie_perdu);
static void update_munition (int b);

//static void gl4duwMouseFunc(void);

/*!\brief opened window width */
static int _windowWidth = 800;
/*!\brief opened window height */
static int _windowHeight = 600;
/*!\brief entier représentant la barre de vie */
static float _lifebar = 100.0;
/*!\brief Quad geometry Id  */
static GLuint _plane = 0;
static GLuint _square = 0;
/*!\brief Sphere geometry Id  */
static GLuint _sphere = 0;
/*!\brief GLSL program Id */
static GLuint _arme_pId = 0;
static GLuint _pId = 0;
/*!\brief plane texture Id */
static GLuint _planeTexId = 0;
static GLuint _planeTexId2 = 0;
/*!\brief sky texture Id */
static GLuint _skyTexId = 0;
/*!\brief imagetexture Id */
static GLuint _immurId = 0;
static GLuint _immurId2 = 0;
static GLuint _importeId = 0;
static GLuint _imchampibadId = 0;
static GLfloat _champiScale = 1;
static GLuint _imtargetId = 0;
static GLuint _imflammeId = 0;
static GLuint _imtexteId = 0;
static GLuint _imsoldatfaceId = 0;
static GLuint _imsoldatdroiteId = 0;
static GLuint _imsoldatgaucheId = 0;
static GLuint _imsoldatdosId = 0;
static GLuint _imsoldat_tireId = 0;
static GLuint _imsoldat_tire2Id = 0;
static GLuint _imarme_pId = 0;
/*!\brief life texture Id */
static GLuint _lifeTexId = 0;
static GLuint _lifeTexId2 = 0;
/*!\brief compass texture Id */
static GLuint _compassTexId = 0;
/*!\brief plane scale factor */
static GLfloat _planeScale = 100.0;
/*!\brief boolean to toggle anisotropic filtering */
static GLboolean _anisotropic = GL_FALSE;
/*!\brief boolean to toggle mipmapping */
static GLboolean _mipmap = GL_FALSE;
/*!\brief boolean to toggle scene fog */
static GLboolean _fog = GL_FALSE;
static GLboolean _boolspace = 0;

GLint _level = 0;
GLboolean _loose = 0;
GLboolean _boolsoldat = 0;

Mix_Music * _musique1;
Mix_Music * _musique2;
Mix_Music * _musique3;
Mix_Chunk * _son_ak47;
Mix_Chunk * _son_recharge;
Mix_Chunk * _son_porte;
Mix_Chunk * _son_cri;
Mix_Chunk * _son_cri_fin;
Mix_Chunk * _son_arme_vide;
Mix_Chunk * _son_tire_ennemy;

GLuint _time_musique1 = 155; // secondes
GLuint _time_musique2 = 200; // secondes
GLuint _time_musique3 = 145; // secondes


SDL_TimerID _timerId;

GLuint _timer_vie_dehors = 0;
GLuint _timer_vie_dehors_p = 0;
GLuint _timer_vie_dedans = 0;
GLuint _timer_vie_dedans_p = 0;
GLuint _timer_chargeur = 0;
GLuint _timer_chargeur_fin = 0;
GLuint _temps_tire_Ennemy = 0;
GLuint _temps_tire_Ennemy_p = 0;

float _cst = 0;

GLfloat _time_musique_playing = 0;
GLuint _tempsPrecedent = 0, _tempsActuel = 0;
char * _timer_char = NULL;

TTF_Font * _police = NULL;
TTF_Font * _police2 = NULL;
TTF_Font * _police3 = NULL;



GLfloat _ryArme = 13;
GLfloat _rzArme = -0.23;
GLboolean _tirArme = 0;
GLboolean _porteBouge = 0;
GLint _boolarme_reload = 0;
GLfloat _xflamme = 0.11;
GLfloat _yflamme = 0.18;
GLfloat _xporteBougetmp = -0.0;
GLfloat _tmp = 0.0;

GLboolean _cheat = 0;

GLuint _muni_max = 120;
GLuint _muni_actuel = 30;

/*!\brief enum that index keyboard mapping for direction commands */
enum kyes_t {
  KLEFT = 0,
  KRIGHT,
  KUP,
  KDOWN
};

/*!\brief virtual keyboard for direction commands */
static GLuint _keys[] = {0, 0, 0, 0};

typedef struct cam_t cam_t;
/*!\brief a data structure for storing camera position and
 * orientation */
struct cam_t {
  GLfloat x, y, z;
  GLfloat theta, sigma;
};

/*!\brief the used camera */
static cam_t _cam = {0, 1, -1, 0, 0};


typedef struct objet objet;

struct objet {
  GLfloat x;
  GLfloat y;
  GLfloat z;
  GLuint id;
};

typedef struct Ennemy Ennemy;

struct Ennemy {
  GLfloat x;
  GLfloat y;
  GLfloat z;
  GLuint idface;
  GLuint iddroite;
  GLuint idgauche;
  GLuint iddos;
  GLuint idtire;
  GLuint idtire2;
  int type;
};

/*!\brief vec champi */
objet * _tabObjet = NULL;
objet * _tabObjet2 = NULL;
Ennemy * _tabEnnemy = NULL;

static int _width_terrain = 31;
static int _height_terrain = 21;
int _nb_objet_sortie = 0;
int dir[4][2] = {{0,1},{1,0},{0,-1},{-1,0}} ;

int _x_init = -2;
int _y_init = -3;

int _nbtabObjet = 0;
int _nbtabObjet2 = 0;
int _nbtabEnnemy = 0;

static void placeallmur (objet * tabobj, int nb);
static void place_ennemy ();
static objet * initObjet (GLfloat z, GLuint id);
static Ennemy * initEnnemy ();
void unEnnemy (GLfloat x, GLfloat z, GLfloat r, GLuint id);

/*!\brief creates the window, initializes OpenGL parameters,
 * initializes data and maps callback functions */
int main(int argc, char ** argv) {
  if(!gl4duwCreateWindow(argc, argv, "GL4Dummies", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         _windowWidth, _windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN))
    return 1;
  //assimpInit("models/M16/SS2-V1.obj");
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  srand(time(NULL));
  assimpInit("models/ak-47/Adding/AK.obj");

  if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1) //Initialisation de l'API Mixer
  {
    printf("%s", Mix_GetError());
  }
  Mix_AllocateChannels(11);

  if(TTF_Init() == -1)
  {
    fprintf(stderr, "Erreur d'initialisation de TTF_Init : %s\n", TTF_GetError());
    exit(EXIT_FAILURE);
  }

  initGL();
  initData();
  atexit(quit);
  gl4duwResizeFunc(resize);
  gl4duwKeyUpFunc(keyup);
  gl4duwKeyDownFunc(keydown);
  gl4duwDisplayFunc(draw);
  gl4duwIdleFunc(idle);
  gl4duwMouseFunc(mouse);

  gl4duwMainLoop();
  return 0;
}

/*!\brief initializes OpenGL parameters :
 *
 * the clear color, enables face culling, enables blending and sets
 * its blending function, enables the depth test and 2D textures,
 * creates the program shader, the model-view matrix and the
 * projection matrix and finally calls resize that sets the projection
 * matrix and the viewport.
 */
static void initGL(void) {
  glClearColor(0.0f, 0.4f, 0.9f, 0.0f);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  _pId  = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
  _arme_pId = gl4duCreateProgram("<vs>shaders/arme.vs", "<fs>shaders/arme.fs", NULL);
  gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  resize(_windowWidth, _windowHeight);
}

/*!\brief initializes data : 
 *
 * creates 3D objects (plane and sphere) and 2D textures.
 */

//ESSAI

GLuint minutes (GLuint t)
{
  int nb = -1, i;
  for (i = 0; i <= t; i += 60)
  {
    nb++;
  }
  return nb;
}

GLuint secondes (GLuint t)
{
  int nb = 0;
  for (nb = t; nb >= 60; nb -= 60);
  return nb;
}

Uint32 refresh(Uint32 intervalle, void *parametre)
{
    //GLfloat time = &parametre; /* Conversion de void* en SDL_Rect* */

    _time_musique_playing -= 0.30;
    return intervalle;
}

//----------
static void initData(void) {
  SDL_Surface * texSurface;
  /* the checkboard texture */
  //GLuint check[] = {-1, 255 << 24, 128 << 24, -1};
  /* a red-white texture used to draw a compass */
  GLuint northsouth[] = {(255 << 24) + 255, -1};
  /* a green and red texture used to draw the lifebar */
  GLuint lifebarG[] = {0x00ff00, -1};
  GLuint lifebarR[] = {0x0000ff, -1};
  /* a fractal texture generated usind a midpoint displacement algorithm */
  GLfloat * hm = gl4dmTriangleEdge(257, 257, 0.4);
  /* generates a quad using GL4Dummies */
  _plane = gl4dgGenQuadf();
  /* generates a sphere using GL4Dummies */
  _sphere = gl4dgGenSpheref(10, 10);
  /* generates a quad using GL4Dummies */
  _square = gl4dgGenQuadf();

  //-------------------------------------
  //Init sons et musiques
  _musique1 = Mix_LoadMUS("sons/1.mp3");
  _musique2 = Mix_LoadMUS("sons/2.mp3");
  _musique3 = Mix_LoadMUS("sons/3.mp3");

  _son_ak47 = Mix_LoadWAV("sons/ak47.wav");
  _son_recharge = Mix_LoadWAV("sons/recharge.wav");
  _son_porte = Mix_LoadWAV("sons/porte.wav");
  _son_cri = Mix_LoadWAV("sons/cris.wav");
  _son_cri_fin = Mix_LoadWAV("sons/cris2.wav");
  _son_arme_vide = Mix_LoadWAV("sons/armevide.wav");
  _son_tire_ennemy = Mix_LoadWAV("sons/tire-enemy.wav");

  int random_music = rand() % 3;
  if(random_music == 0)
  {
    _time_musique_playing = (GLfloat) _time_musique1;
    Mix_PlayMusic(_musique1, 0);
  }
  if(random_music == 1)
  {
    _time_musique_playing = (GLfloat) _time_musique2;
    Mix_PlayMusic(_musique2, 0);    
  }
  if(random_music == 2)
  {
    _time_musique_playing = (GLfloat) _time_musique3;
    Mix_PlayMusic(_musique3, 0);
  }

  _cst = _time_musique_playing / 100;
  //printf("%f\n", _cst);
  //Fin Init sons et musiques
  //-------------------------------------
  _tempsActuel = SDL_GetTicks();
  //_timerId = SDL_AddTimer(30, refresh, &_time_musique_playing);
  //TIMER
  //-------------------------------------
  
  //FIN TIMER
  //-------------------------------------

  /* creation and parametrization of the plane texture */
  glGenTextures(1, &_planeTexId);
  glBindTexture(GL_TEXTURE_2D, _planeTexId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, check);
  if( (texSurface = IMG_Load("img/sol.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  glGenTextures(1, &_planeTexId2);
  glBindTexture(GL_TEXTURE_2D, _planeTexId2);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, check);
  if( (texSurface = IMG_Load("img/sol2.jpg")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSurface->w, texSurface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  /* creation and parametrization of the sky texture */
  glGenTextures(1, &_skyTexId);
  glBindTexture(GL_TEXTURE_2D, _skyTexId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 257, 257, 0, GL_RED, GL_FLOAT, hm);
  free(hm);

  /* creation and parametrization of the compass texture */
  glGenTextures(1, &_compassTexId);
  glBindTexture(GL_TEXTURE_2D, _compassTexId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, northsouth);

  /* creation and parametrization of the LIFE texture */
  glGenTextures(1, &_lifeTexId);
  glBindTexture(GL_TEXTURE_2D, _lifeTexId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, lifebarG);

  glGenTextures(1, &_lifeTexId2);
  glBindTexture(GL_TEXTURE_2D, _lifeTexId2);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, lifebarR);

  /* creation and parametrization of the champi texture */
  

  glGenTextures(1, &_immurId);
  glBindTexture(GL_TEXTURE_2D, _immurId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/mur.jpg")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSurface->w, texSurface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);


  glGenTextures(1, &_immurId2);
  glBindTexture(GL_TEXTURE_2D, _immurId2);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/mur4.jpg")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSurface->w, texSurface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);


  //-----------------------------------------
  //Texture pour le texte
  glGenTextures(1, &_imtexteId);
  glBindTexture(GL_TEXTURE_2D, _imtexteId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/transparent.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  // //SDL_FillRect(texSurface, NULL, 0x000000);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  glBindTexture(GL_TEXTURE_2D, _imtexteId);
  SDL_Surface * texte = NULL;
  SDL_Rect position;
  _police = TTF_OpenFont("fonts/arialbold.ttf", 25);
  if(!_police) 
  {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    exit(1);
  }
  _police2 = TTF_OpenFont("fonts/sanglant.ttf", 13);
  if(!_police2) 
  {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    exit(1);
  }
  _police3 = TTF_OpenFont("fonts/arialbold.ttf", 8);
  if(!_police3) 
  {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    exit(1);
  }
  _timer_char = malloc(25 * sizeof _timer_char[0]);
  sprintf(_timer_char, "%d:%d:00", (int)minutes(_time_musique_playing), (int)secondes(_time_musique_playing));
  SDL_Color couleur_rouge_un_peu_pale = {255, 25, 25, 0};
  texte = TTF_RenderText_Blended(_police, _timer_char, couleur_rouge_un_peu_pale);

  position.x = texSurface->w / 2 - texte->w;
  position.y = texSurface->h / 2 - texte->h;
  position.w = texte->w;
  position.h = texte->h;
  SDL_BlitSurface(texte, NULL, texSurface, &position); /* Blit du texte */
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  // glGenerateMipmap(GL_TEXTURE_2D);
  //Fin texture pour le texte

  //----------------------------------------------
  /* creation and parametrization of the champidore texture */
  glGenTextures(1, &_importeId);
  glBindTexture(GL_TEXTURE_2D, _importeId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/porte.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  /* creation and parametrization of the champibad texture */
  // glGenTextures(1, &_imchampibadId);
  // glBindTexture(GL_TEXTURE_2D, _imchampibadId);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  // if( (texSurface = IMG_Load("img/champibad.png")) == NULL ) {
  //   fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
  //   exit(1);
  // }
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  // glGenerateMipmap(GL_TEXTURE_2D);

  /* creation and parametrization of the target texture */
  glGenTextures(1, &_imtargetId);
  glBindTexture(GL_TEXTURE_2D, _imtargetId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/target.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  /* creation and parametrization of the flamme texture */
  glGenTextures(1, &_imflammeId);
  glBindTexture(GL_TEXTURE_2D, _imflammeId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/flamme2.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  /* image arme pour coller par dessus l'objet 3D evite le mipmapping*/

  glGenTextures(1, &_imarme_pId);
  glBindTexture(GL_TEXTURE_2D, _imarme_pId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/arme.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  /* Texture du soldat */
  //----------------------------------------------------------------
  glGenTextures(1, &_imsoldatfaceId);
  glBindTexture(GL_TEXTURE_2D, _imsoldatfaceId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/soldat-bouge.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  //----------------------------------------------------------------
  glGenTextures(1, &_imsoldatdroiteId);
  glBindTexture(GL_TEXTURE_2D, _imsoldatdroiteId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/soldat-droite.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  //----------------------------------------------------------------
  glGenTextures(1, &_imsoldatgaucheId);
  glBindTexture(GL_TEXTURE_2D, _imsoldatgaucheId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/soldat-gauche.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  //----------------------------------------------------------------
  glGenTextures(1, &_imsoldatdosId);
  glBindTexture(GL_TEXTURE_2D, _imsoldatdosId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/soldat-dos.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);

  //----------------------------------------------------------------
  glGenTextures(1, &_imsoldat_tireId);
  glBindTexture(GL_TEXTURE_2D, _imsoldat_tireId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/soldat-tire.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);
  //-----------------------------------------------------------------
  glGenTextures(1, &_imsoldat_tire2Id);
  glBindTexture(GL_TEXTURE_2D, _imsoldat_tire2Id);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/soldat-tire2.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);


  /* End of init texture */ 
  glBindTexture(GL_TEXTURE_2D, 0);

  //Init les champigons dans un vecteur
  _tabObjet = initObjet(1, _immurId);
  _tabObjet2 = initObjet(-1, _immurId2);
  _tabEnnemy = initEnnemy();

}

/*!\brief function called by GL4Dummies' loop at resize. Sets the
 *  projection matrix and the viewport according to the given width
 *  and height.
 * \param w window width
 * \param h window height
 */
static void resize(int w, int h) {
  _windowWidth = w; 
  _windowHeight = h;
  glViewport(0, 0, _windowWidth, _windowHeight);
  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  //gl4duFrustumf(-0.5, 0.5, -0.5 * _windowHeight / _windowWidth, 0.5 * _windowHeight / _windowWidth, 0.25, _planeScale + 1.0);
  gl4duPerspectivef(90.0, (double) _windowWidth / (double) _windowHeight, 0.1, 1000);
  gl4duBindMatrix("modelViewMatrix");
}

/*!\brief function called by GL4Dummies' loop at idle.
 * 
 * uses the virtual mouse states to move the camera according to
 * direction, orientation and time (dt = delta-time)
 */

// unObjet(t[i].x, t[i].y, 0, t[i].id);
// unObjet(t[i].x, t[i].y+2, 0, t[i].id);
// unObjet(t[i].x+1, t[i].y+1, 90, t[i].id);
// unObjet(t[i].x-1, t[i].y+1, 90, t[i].id);
int collision (GLfloat x, GLfloat y, GLfloat z)
{
  if(_level == 0)
  {
    if(_cheat == 1)
      return 0;
    int i;
    for (i = 0; i < _nbtabObjet; ++i)
    {
      if(_tabObjet[i].id == _importeId)
      {
        if(x <= _tabObjet[i].x + 1 && x >= _tabObjet[i].x - 1 && z <= _tabObjet[i].y + 0.3 && z >= _tabObjet[i].y - 0.3)
          return 1;
      }
      //printf("PAS COLLISION : %f <= %f && %f >= %f && %f <= %f && %f >= %f\n", _cam.x, _tabObjet[i].x + 1.005, x, _tabObjet[i].x - 1.005, y, _tabObjet[i].y + 1.005, y, _tabObjet[i].y - 1.005);
      if(x <= _tabObjet[i].x + 1.3 && x >= _tabObjet[i].x - 1.3 && z <= _tabObjet[i].y + 1.3 && z >= _tabObjet[i].y - 1.3 && (_tabObjet[i].id == _immurId || _tabObjet[i].id == _immurId2))
      {
        //printf("%f <= %f && %f >= %f && %f <= %f && %f >= %f\n", x, _tabObjet[i].x + 1.005, x, _tabObjet[i].x - 1.005, y, _tabObjet[i].y + 1.005, y, _tabObjet[i].y - 1.005);
        if(_tabObjet[i].id == _immurId || _tabObjet[i].id == _immurId2)
        {
          return 1;
        }
        else if(_tabObjet[i].id == _importeId)
        {
          return 2;
        }
        else if(_tabObjet[i].id == _imchampibadId)
        {
          return 3;
        }
        else
          return 4;
      }
    }
  }
  else if(_level == 1)
  {
    if(_cheat == 1)
      return 0;
    int i;
    for (i = 0; i < _nbtabObjet2; ++i)
    {
      if(_tabObjet2[i].id == _importeId)
      {
        if(x <= _tabObjet2[i].x + 1 && x >= _tabObjet2[i].x - 1 && z <= _tabObjet2[i].y + 0.3 && z >= _tabObjet2[i].y - 0.3)
          return 1;
      }
      //printf("PAS COLLISION : %f <= %f && %f >= %f && %f <= %f && %f >= %f\n", _cam.x, _tabObjet2[i].x + 1.005, x, _tabObjet2[i].x - 1.005, y, _tabObjet2[i].y + 1.005, y, _tabObjet2[i].y - 1.005);
      if(x <= _tabObjet2[i].x + 1.3 && x >= _tabObjet2[i].x - 1.3 && z <= _tabObjet2[i].y + 1.3 && z >= _tabObjet2[i].y - 1.3 && (_tabObjet2[i].id == _immurId || _tabObjet2[i].id == _immurId2))
      {
        //printf("%f <= %f && %f >= %f && %f <= %f && %f >= %f\n", x, _tabObjet2[i].x + 1.005, x, _tabObjet2[i].x - 1.005, y, _tabObjet2[i].y + 1.005, y, _tabObjet2[i].y - 1.005);
        if(_tabObjet2[i].id == _immurId || _tabObjet2[i].id == _immurId2)
        {
          return 1;
        }
        else if(_tabObjet2[i].id == _importeId)
        {
          return 2;
        }
        else if(_tabObjet2[i].id == _imchampibadId)
        {
          return 3;
        }
        else
          return 4;
      }
    }
  }

  return 0;
}

static void mouse(int button, int state, int x, int y) 
{
  if(_loose != 1)
  {
    if(!state) // bouton relacher
      ;
    else if(button == GL4D_BUTTON_LEFT && _muni_actuel > 0 && _boolarme_reload == 0) 
    {
      Mix_PlayChannel(1, _son_ak47, 0);
      _tirArme = 1;
      _muni_actuel--;
    }
    else if(button == GL4D_BUTTON_LEFT)
    {
      Mix_PlayChannel(8, _son_arme_vide, 0);
    }
    if(button == GL4D_BUTTON_RIGHT) 
    {

    }
  }
}


static void idle(void) {
  int xm, ym;
  double dt, dtheta = M_PI, step = 5.0;
  static Uint32 t0 = 0, t;
  dt = ((t = SDL_GetTicks()) - t0) / 1000.0;
  t0 = t;
  GLfloat xtmp = _cam.x, ytmp = _cam.y, ztmp = _cam.z;

  if(_boolspace == 1)
  {
    gl4duwKeyDownFunc(keydown2);
    if(_cam.y <= 2)
      _cam.y += 0.005;
    else
      _boolspace = 0;
  }
  if(_boolspace == 0)
  {
    gl4duwKeyDownFunc(keydown);
    if(_cam.y > 1)
      _cam.y -= 0.005;
  }

  if(_keys[KLEFT])
  {
    xtmp = _cam.x + -dt * step * sin(_cam.theta + M_PI/2);
    ztmp = _cam.z + -dt * step * cos(_cam.theta + M_PI/2);
    if(collision(xtmp, ytmp, ztmp) == 0)
    {
      _cam.x = xtmp;
      _cam.z = ztmp;
    }
  }
  if(_keys[KRIGHT])
  {
    xtmp = _cam.x + -dt * step * sin(_cam.theta - M_PI/2);
    ztmp = _cam.z + -dt * step * cos(_cam.theta - M_PI/2);
    if(collision(xtmp, ytmp, ztmp) == 0)
    {
      _cam.x = xtmp;
      _cam.z = ztmp;
    }
  }
  if(_keys[KUP]) 
  {
    xtmp = _cam.x + -dt * step * sin(_cam.theta);
    ztmp = _cam.z + -dt * step * cos(_cam.theta);
    if(collision(xtmp, ytmp, ztmp) == 0)
    {
      _cam.x = xtmp;
      _cam.z = ztmp;
    }
  }
  if(_keys[KDOWN]) 
  {
    xtmp = _cam.x + dt * step * sin(_cam.theta);
    ztmp = _cam.z + dt * step * cos(_cam.theta);
    if(collision(xtmp, ytmp, ztmp) == 0)
    {
      _cam.x = xtmp;
      _cam.z = ztmp;
    }
  }


  //Gestion de la souris avec la caméra
  if(_loose != 1)
  {
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_PumpEvents();
    SDL_GetRelativeMouseState(&xm, &ym);
  }

  double diff;                          

  //test si en dehors de limite au démarrage
  if(_cam.sigma < - ((M_PI / 2) - 0.4) - 0.1)
    _cam.sigma = 0;
  if(_cam.sigma > (M_PI / 2) - 0.4 + 0.1)
    _cam.sigma = 0;
  //fin test
  
  if(xm < 0)
  {
    diff = (double) xm / (double) _windowWidth;
    diff += 0.001;

    _cam.theta += dt * dtheta + diff;
  }
  if(xm > 0)
  {
    diff = (double) xm / (double) _windowWidth;
    diff += -0.001;

    _cam.theta -= dt * dtheta + diff;
  }
  
  if(ym < 0 && _cam.sigma > - ((M_PI / 2) - 0.4))
  {
    diff = (double) ym / (double) _windowWidth;
    diff += 0.001;

    _cam.sigma -= dt * dtheta + diff;
    //printf("%f\n", _cam.sigma);
  }
  if(ym > 0 && _cam.sigma < (M_PI / 2) - 0.4)
  {
    diff = (double) ym / (double) _windowWidth;
    diff += -0.001;

    _cam.sigma += dt * dtheta + diff;
    //printf("%f\n", _cam.sigma);
  }
}

/*!\brief function called by GL4Dummies' loop at key-down (key
 * pressed) event.
 * 
 * stores the virtual keyboard states (1 = pressed) and toggles the
 * boolean parameters of the application.
 */
static void keydown(int keycode) {
  double dt, dtheta = M_PI;
  static Uint32 t0 = 0, t;
  dt = ((t = SDL_GetTicks()) - t0) / 1000.0;
  t0 = t;
  GLint v[2];

  switch(keycode) {
  case SDLK_LEFT:
    _keys[KLEFT] = 1;
    break;
  case SDLK_RIGHT:
    _keys[KRIGHT] = 1;
    break;
  case SDLK_UP:
    _keys[KUP] = 1;
    break;
  case SDLK_DOWN:
    _keys[KDOWN] = 1;
    break;
  case 'q':
    _keys[KLEFT] = 1;
    break;
  case 'd':
    _keys[KRIGHT] = 1;
    break;
  case 'z':
    _keys[KUP] = 1;
    break;
  case 's':
    _keys[KDOWN] = 1;
    break;
  case SDLK_SPACE:
    if(_boolspace == 0 && _cam.y <= 1)
      _boolspace = 1;
    break;
  case 't':
    if(_rzArme >= -0.23)
    {
      Mix_PlayChannel(1, _son_ak47, 0);
      _tirArme = 1;
    }
    break;
  case 'r':
    update_munition(1);
    break;
  case 'e':
  case SDLK_RETURN:
    Mix_PlayChannel(2, _son_porte, 0);
    _porteBouge = 1;
    break;
  case SDLK_ESCAPE:
  case 'x':
    exit(0);
    /* when 'w' pressed, toggle between line and filled mode */
  case 'w':
    glGetIntegerv(GL_POLYGON_MODE, v);
    if(v[0] == GL_FILL) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glLineWidth(3.0);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glLineWidth(1.0);
    }
    break;
  case 'f':
    _fog = !_fog;
    break;
  case 'j':
  {
    _cam.theta -= dt * dtheta;
    break;
  }
  case 'l':
  {
    _cam.theta += dt * dtheta;
    break;
  }
  case 'c':
  {
    if(_cheat == 0)
      _cheat = 1;
    else
      _cheat = 0;
    break;
  }
    /* when 'm' pressed, toggle between mipmapping or nearest for the plane texture */
  case 'm': {
    _mipmap = !_mipmap;
    glBindTexture(GL_TEXTURE_2D, _planeTexId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    break;
  }
    /* when 'a' pressed, toggle on/off the anisotropic mode */
  case 'a': {
    _anisotropic = !_anisotropic;
    /* l'Anisotropic sous GL ne fonctionne que si la version de la
       bibliothèque le supporte ; supprimer le bloc ci-après si
       problème à la compilation. */
#ifdef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    GLfloat max;
    glBindTexture(GL_TEXTURE_2D, _planeTexId);
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, _anisotropic ? max : 1.0f);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
    break;
  }
  default:
    break;
  }
}

void restart ()
{
  _loose = 0;
  _lifebar = 100;
  _cam.x = 0;
  _cam.y = 1;
  _cam.z = -1;
  _cam.theta = 0;
  _cam.sigma = 0;
  gl4duwKeyDownFunc(keydown);
}

static void keydown2(int keycode) {
  switch(keycode) {
    case 'r':
    {
      restart();
      break;
    }
    case SDLK_ESCAPE:
    {  
      exit(0);
    }
    default:
    break;
  }
}


/*!\brief function called by GL4Dummies' loop at key-up (key
 * released) event.
 * 
 * stores the virtual keyboard states (0 = released).
 */
static void keyup(int keycode) {
  switch(keycode) {
  case SDLK_LEFT:
    _keys[KLEFT] = 0;
    break;
  case SDLK_RIGHT:
    _keys[KRIGHT] = 0;
    break;
  case SDLK_UP:
    _keys[KUP] = 0;
    break;
  case SDLK_DOWN:
    _keys[KDOWN] = 0;
    break;
  case 'q':
    _keys[KLEFT] = 0;
    break;
  case 'd':
    _keys[KRIGHT] = 0;
    break;
  case 'z':
    _keys[KUP] = 0;
    break;
  case 's':
    _keys[KDOWN] = 0;
    break;
  default:
    break;
  }
}

void unEnnemy (GLfloat x, GLfloat z, GLfloat r, GLuint id) 
{
  glEnable(GL_CULL_FACE);
  //glCullFace(GL_FRONT);
  gl4duPushMatrix(); {
    gl4duTranslatef(x, 0.8, z);
    if(r != 0)
      gl4duRotatef(r, 0, 1, 0);
    gl4duScalef(0.8, 0.8, 0.8);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glCullFace(GL_BACK);
  glBindTexture(GL_TEXTURE_2D, id);
  glUniform1f(glGetUniformLocation(_pId, "texRepeat"), 1.0);
  glUniform1i(glGetUniformLocation(_pId, "sky"), 0);
  gl4dgDraw(_square);
  //glDisable(GL_DEPTH_TEST);
  
}


void unObjet(GLfloat x, GLfloat z, GLfloat y, GLfloat r, GLuint id) 
{
  //glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CULL_FACE);

  gl4duPushMatrix(); {
    gl4duTranslatef(x, y, z);
    if(r != 0)
      gl4duRotatef(r, 0, 1, 0);
    gl4duScalef(_champiScale, _champiScale, 1);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glCullFace(GL_BACK);
  glBindTexture(GL_TEXTURE_2D, id);
  glUniform1f(glGetUniformLocation(_pId, "texRepeat"), 1.0);
  glUniform1i(glGetUniformLocation(_pId, "sky"), 0);
  gl4dgDraw(_square);
  //glDisable(GL_DEPTH_TEST);
  //glDisable(GL_CULL_FACE);
}

void genererVide(int * laby){
  int i,j;
  for(i=0;i<_height_terrain;i++){
    for(j=0;j<_width_terrain;j++){
      if(i%2==0 || j%2==0){
        *laby= -1 ;
      }
      else{
        *laby = (i/2)*(_height_terrain/2) + (j/2);
      }
      laby++ ;
    }
  }
}

void propager(int val, int x, int y, int * laby){
  int i;
  laby[y*(_width_terrain) + x] = val ;
  //printf("a\n") ;
  for(i=0;i<4;i++){
    //printf("boucle, %d\n",laby[(y+dir[i][1])*(_width_terrain)  +  (x+dir[i][0])]) ;
    if( laby[(y+dir[i][1])*(_width_terrain)  +  (x+dir[i][0])] > val ){
      propager(val,x+dir[i][0],y+dir[i][1],laby) ;
    }
  }
}


void genererLaby(int * laby,int n){
  int x, y ;
  if(n==((_height_terrain/2)*(_width_terrain/2)-1)){
    return ;
  }
  x=rand()%(_width_terrain-2)+1 ;
  if(x%2==0){
    y=(rand()%((_height_terrain-1)/2))*2+1 ;
    if(laby[y*_width_terrain+x]!=-1){
      genererLaby(laby,n) ;
    }
    else if(laby[y*_width_terrain +x-1]==laby[y*_width_terrain +x+1]){
      genererLaby(laby,n) ;
    }
    else if(laby[y*_width_terrain +x-1]<laby[y*_width_terrain +x+1]){
      propager(laby[y*_width_terrain +x-1],x,y,laby) ;
      genererLaby(laby,n+1) ;
    }
    else{
      propager(laby[y*_width_terrain +x+1],x,y,laby) ;
      genererLaby(laby,n+1) ;
    }
  }
  else{
    y=(rand()%((_height_terrain-3)/2))*2+2 ;
    if(laby[y*_width_terrain+x]!=-1){
      genererLaby(laby,n) ;
    }
    else if(laby[y*_width_terrain +x-_width_terrain]==laby[y*_width_terrain +x+_width_terrain]){
      genererLaby(laby,n) ;
    }
    else if(laby[y*_width_terrain +x-_width_terrain]<laby[y*_width_terrain +x+_width_terrain]){
      propager(laby[y*_width_terrain +x-_width_terrain],x,y,laby) ;
      genererLaby(laby,n+1) ;
    }
    else{
      propager(laby[y*_width_terrain +x+_width_terrain],x,y,laby) ;
      genererLaby(laby,n+1) ;
    }
  }
}

Ennemy * initEnnemy ()
{
  Ennemy * t;
  t = malloc (20*sizeof t[0]);

  t[_nbtabEnnemy].x = 0;
  t[_nbtabEnnemy].y = -8;
  t[_nbtabEnnemy].z = 0;
  t[_nbtabEnnemy].idface = _imsoldatfaceId;
  t[_nbtabEnnemy].iddroite = _imsoldatdroiteId;
  t[_nbtabEnnemy].idgauche = _imsoldatgaucheId;
  t[_nbtabEnnemy].iddos = _imsoldatdosId;
  t[_nbtabEnnemy].idtire = _imsoldat_tireId;
  t[_nbtabEnnemy].idtire2 = _imsoldat_tire2Id;
  t[_nbtabEnnemy].type = 1;
  _nbtabEnnemy++;

  return t;
}

int ligne(float xi,float yi,float xf,float yf) 
{
  float x,y ;
  float a,b ;
  //printf("%f - %f\n", xf, yf);
  a = (yf-yi)/(xf-xi) ;
  b = yi - a * xi ;
  for ( x = xi ; x <= xf ; x += 0.01) 
  {
    y = (a * x + b) ;
    if(collision(x, _cam.y, y))
      return 0;
  }
  return 1;
}

float diff_abs (float x, float x2)
{
  float a;
  a = x - x2;
  if(a > 0)
    return a;
  return -a;
}

GLboolean vision_ennemy (int type, GLfloat x, GLfloat y, GLfloat z)
{
  double xtmp = _cam.x, ytmp = _cam.z;
  int rayon_vision;

  if(type == 1)
    rayon_vision = 5;

  float diff = diff_abs(x, xtmp);
  float diff2 = diff_abs(z, ytmp);
  //printf("%f - %f /// %f - %f\n", x, _cam.x, z, _cam.z);
  if(diff <= rayon_vision && diff2 <= rayon_vision)
    if(ligne(xtmp, ytmp, x, z))
      return 1;
  return 0;
}

void place_ennemy ()
{
  _temps_tire_Ennemy = SDL_GetTicks();
  int i;
  for (i = 0; i < _nbtabEnnemy; ++i)
  {
    if(vision_ennemy(_tabEnnemy[i].type, _tabEnnemy[i].x, _cam.y, _tabEnnemy[i].y) && _cheat != 1 && _loose != 1) // a modifier : le _cam.y
    {
      
      if (_temps_tire_Ennemy - _temps_tire_Ennemy_p > 800) /* Si 30 ms se sont écoulées depuis le dernier tour de boucle */
      {
        
        _temps_tire_Ennemy_p = _temps_tire_Ennemy;
        if(_boolsoldat == 0)
        {
          Mix_PlayChannel(9, _son_tire_ennemy, 0);
          loosepdv(25);
          _boolsoldat = 1;
        }
        else
          _boolsoldat = 0;
      }
      if(_boolsoldat == 0)
      { 
        unEnnemy(_tabEnnemy[i].x, _tabEnnemy[i].y, 0, _tabEnnemy[i].idtire);
      }
      else
      { 
        unEnnemy(_tabEnnemy[i].x, _tabEnnemy[i].y, 0, _tabEnnemy[i].idtire2);
      }
    }
    else
    {
      unEnnemy(_tabEnnemy[i].x, _tabEnnemy[i].y, 0, _tabEnnemy[i].idface);
      unEnnemy(_tabEnnemy[i].x, _tabEnnemy[i].y, -90, _tabEnnemy[i].iddroite);
      unEnnemy(_tabEnnemy[i].x, _tabEnnemy[i].y, 90, _tabEnnemy[i].idgauche);
      unEnnemy(_tabEnnemy[i].x, _tabEnnemy[i].y, 180, _tabEnnemy[i].iddos);
    }
  }
}

objet * initObjet (GLfloat z, GLuint id_mur)
{
  int i, j, id = 0, nb = 0;
  int * laby ;
  objet * t;

  laby = malloc(_width_terrain*_height_terrain*sizeof(int));
  t = malloc (_width_terrain*_height_terrain*sizeof t[0]);

  int ** plaby = &laby;
  genererVide(*plaby) ;
  genererLaby(*plaby,0);

  // for (i = 0; i < _height_terrain; ++i)
  // {
  //   for (j = 0; j < _width_terrain; ++j)
  //   {
  //     printf(" %d ", laby[i*_width_terrain + j]);
  //     //laby++;
  //   }
  //   printf("\n");
  // }

  int ytest;
  int xtest;

  for (i = 0, ytest = _y_init; i < _height_terrain; ++i, ytest += 2)
  {
    for (j = 0, xtest = _x_init; j < _width_terrain; ++j, xtest += 2)
    {
      //printf("I = %d -- J = %d -- _height_terrain = %d -- _width_terrain = %d\n", i, j, _height_terrain, _width_terrain);
      //Placer les 2 portes entrées et sortie
      if(i == 0 && j == 1)
      {
        laby[i * _width_terrain + j] = 1;
      }
      if(i == _height_terrain-1 && j == _width_terrain-2)
      {
        _nb_objet_sortie = nb;
        laby[i * _width_terrain + j] = 1;
      }
      //fprintf(stderr, "TEST2 -- plaby[%d][%d] : %d\n", i, j, plaby[i][j]);
      if(laby[i * _width_terrain + j] == -1)
        id = id_mur;
      else if (laby[i * _width_terrain + j] == 1)
        id = _importeId;
      if(id != 0)
      {
        t[nb].x = xtest;
        t[nb].y = ytest;
        t[nb].z = z;
        t[nb].id = id;
        //printf("t[%d].x = %d -- t[%d].z = %d -- t[%d] = %d\n", nb, t[nb].x, nb, t[nb].z, nb, id);
        nb++;

        id = 0;
      }
      //laby++;
    }
  }
  if(z == 1)
    _nbtabObjet = nb;
  else
    _nbtabObjet2 = nb;

  _y_init = ytest;
  _x_init = xtest-6;

  return t;
}

void placeallmur (objet * tabobj, int nb)
{
  int i;

  for (i = 0; i < nb; ++i)
  {
    if(tabobj[i].id == _immurId || tabobj[i].id == _immurId2)
    {
      unObjet(tabobj[i].x, tabobj[i].y -1, tabobj[i].z, 0, tabobj[i].id);
      unObjet(tabobj[i].x -1, tabobj[i].y, tabobj[i].z, 90, tabobj[i].id);
      unObjet(tabobj[i].x, tabobj[i].y +1, tabobj[i].z, 0, tabobj[i].id);
      unObjet(tabobj[i].x +1, tabobj[i].y, tabobj[i].z, 90, tabobj[i].id);
    }
    if(tabobj[i].id == _importeId)
      unObjet(tabobj[i].x, tabobj[i].y, tabobj[i].z, 0, tabobj[i].id);
  }
}

void deplaceUnObjet (objet * tabobj, int nb)
{
  int i;
  for (i = 0; i < nb; ++i)
  {
    if(_cam.x > tabobj[i].x - 2 - _tmp && _cam.x < tabobj[i].x + 2 +_tmp && _cam.z < tabobj[i].y + 2 + _tmp && _cam.z > tabobj[i].y - 2 - _tmp)
      {
        if(tabobj[i].id == _importeId)
        {
          //printf("Vous êtes à proximité d'une porte ! i = %d\n", i);
          if(_porteBouge == 0)
          {
            _tmp = 4.0;// Determine la distance jusqu'à laquelle on se trouve pour que la porte continuer de s'ouvrir
            _xporteBougetmp = tabobj[i].x; // valeur x d'origine de la porte.
          }
          if(_porteBouge == 1)
          {
            if(tabobj[i].x > _xporteBougetmp + 2.0)
            {
              _porteBouge = 0; // Booléen pour dire si oui ou non la porte bouge
            }
            else
            {
              tabobj[i].x += 0.016;
            }
          }
        }
      }
  }
}

void update_munition (int b)
{
  if(b == 0) //fin de chargeur
  {
    if (_timer_chargeur - _timer_chargeur_fin > 1400)
    {
      if(_muni_actuel <= 0)
      {
        _boolarme_reload = 1;
        _timer_chargeur_fin = _timer_chargeur;
        Mix_PlayChannel(7, _son_recharge, 1);
        if(_muni_max > 30)
        {
          _muni_actuel += 30;
          _muni_max -= 30;
        }
        else if(_muni_max <= 30 && _muni_max > 0)
        {
          _muni_actuel += _muni_max;
          _muni_max = 0;
        }
      }
    }
  }
  else // appuie sur la touche 'r'
  {
    if (_timer_chargeur - _timer_chargeur_fin > 1400 && _muni_actuel != 30)
    {
      _boolarme_reload = 1;
      _timer_chargeur_fin = _timer_chargeur;
      Mix_PlayChannel(7, _son_recharge, 1);
      if(_muni_max >= 30)
      {
        _muni_max += _muni_actuel;
        _muni_max -= 30;
        _muni_actuel = 30; 
      }
      else if(_muni_max < 30 && _muni_max > 0)
      {
        while(_muni_actuel < 30)
        {
          _muni_max--;
          _muni_actuel++;
        }
      }
    }
  }
}

void refresh_texture_timer_et_update_munition ()
{
  int m, s;
  SDL_Surface * texSurface;
  glGenTextures(1, &_imtexteId);
  glBindTexture(GL_TEXTURE_2D, _imtexteId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  if( (texSurface = IMG_Load("img/transparent.png")) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", SDL_GetError());
    exit(1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  glBindTexture(GL_TEXTURE_2D, _imtexteId);
  SDL_Surface * texte = NULL;
  SDL_Rect position;
  
  //munition
  if(_loose == 0)
  {
    sprintf(_timer_char, "%d / %d", _muni_actuel, _muni_max);
    SDL_Color couleur_noir = {0, 0, 0, 1};
    texte = TTF_RenderText_Blended(_police3, _timer_char, couleur_noir);
  
    position.x = texSurface->w / 2 - texte->w - 110;
    position.y = texSurface->h / 2 - texte->h + 79;
    position.w = texte->w;
    position.h = texte->h;
    SDL_BlitSurface(texte, NULL, texSurface, &position); /* Blit du texte */
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  }

  //timer
  if(_loose == 1)
  {
    _timer_char = "Vous avez perdu !";
    SDL_Color couleur_rouge_un_peu_pale = {255, 25, 25, 0};
    texte = TTF_RenderText_Blended(_police2, _timer_char, couleur_rouge_un_peu_pale);
  
    position.x = texSurface->w / 2 - texte->w + 35;
    position.y = texSurface->h / 2 - texte->h + 39;
    position.w = texte->w;
    position.h = texte->h;
    SDL_BlitSurface(texte, NULL, texSurface, &position); /* Blit du texte */
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  }
  else
  {
    m = (int)minutes(_time_musique_playing);
    s = (int)secondes(_time_musique_playing);
    if(s < 10)
      sprintf(_timer_char, "0%d:0%d:00", m, s);
    else
      sprintf(_timer_char, "0%d:%d:00", m, s);
    SDL_Color couleur_rouge_un_peu_pale = {255, 25, 25, 0};
    texte = TTF_RenderText_Blended(_police, _timer_char, couleur_rouge_un_peu_pale);
  
    position.x = texSurface->w / 2 - texte->w;
    position.y = texSurface->h / 2 - texte->h;
    position.w = texte->w;
    position.h = texte->h;
    SDL_BlitSurface(texte, NULL, texSurface, &position); /* Blit du texte */
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
  }  
}

void timer()
{
  _timer_vie_dehors = SDL_GetTicks();
  _timer_vie_dedans = SDL_GetTicks();
  _timer_chargeur = SDL_GetTicks();
  _tempsActuel = SDL_GetTicks();
  if (_tempsActuel - _tempsPrecedent > 300) /* Si 30 ms se sont écoulées depuis le dernier tour de boucle */
  {
    _time_musique_playing -= 0.30;
    _tempsPrecedent = _tempsActuel; /* Le temps "actuel" devient le temps "precedent" pour nos futurs calculs */
    refresh_texture_timer_et_update_munition();
  }

  if (_timer_vie_dehors - _timer_vie_dehors_p > _cst + 15000 && _loose != 1)
  {
    printf("Dépêchez-vous, le poison vous infecte !\n");
    _timer_vie_dehors_p = _timer_vie_dehors;
    loosepdv(_cst + 25);
  }

  gl4duBindMatrix("projectionMatrix");
  gl4duPushMatrix(); {
    gl4duLoadIdentityf();
    gl4duBindMatrix("modelViewMatrix");
    gl4duPushMatrix(); {
      gl4duLoadIdentityf();
      gl4duTranslatef(0.50, 0.5, -0.1);
      gl4duScalef(1.8, 1.8, 1.8);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    gl4duBindMatrix("projectionMatrix");
  } gl4duPopMatrix();
  gl4duBindMatrix("modelViewMatrix");
  glBindTexture(GL_TEXTURE_2D, _imtexteId);
  /* texture repeat only once */
  glUniform1f(glGetUniformLocation(_pId, "texRepeat"), 1);
  /* tells pId that the sky is false */
  gl4dgDraw(_plane);
}


void draw_arme ()
{
  GLfloat lum[4] = {0.0, 0.0, 5.0, 1.0};
  
  if(_boolarme_reload == 1)
  {
    if(_rzArme > -0.6)
      _rzArme -= 0.015;
    if(_rzArme <= -0.6)
      _boolarme_reload = 2;
  }
  if(_boolarme_reload == 2)
  {
    if(_rzArme < -0.23)
    {
      _rzArme += 0.015;
    }
    else
      _boolarme_reload = 0;
  }

  if(_tirArme == 1)
  {
    _ryArme += 2;
    if(_ryArme > 32)
    { 
      _tirArme = 0;
      _ryArme = 13;
    }
  }
  printf("%f _ %f \n", _rzArme, _ryArme);
  glUseProgram(_arme_pId);
  glUniform4fv(glGetUniformLocation(_pId, "lumpos"), 1, lum);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  gl4duPushMatrix(); {
    gl4duLoadIdentityf();
    gl4duScalef(2, 2, 2);
    gl4duTranslatef(0.25, _rzArme, -0.05);
    gl4duRotatef(90, 0, 1, 0);
    gl4duRotatef(_ryArme, 0, 0, 1);
    assimpDrawScene();
  } gl4duPopMatrix();
}

GLboolean _flash = 0;
GLboolean _boolanimationfinlevel = 0;
void loosepdv (float vie_perdu)
{
  //printf("%f\n", _lifebar);
  _lifebar -= vie_perdu;
  _flash = 25;
  Mix_PlayChannel(3, _son_cri, 0);
}

void ifgagne ()
{
  if(_cam.x > _tabObjet[_nb_objet_sortie].x - 4 && _cam.x < _tabObjet[_nb_objet_sortie].x + 4 && _cam.z > _tabObjet[_nb_objet_sortie].y + 1 && _level == 0)
  {
    printf("Vous avez gagné.\n");
    _level = 1;
    _boolanimationfinlevel = 1;
    GLuint tmp = _planeTexId;
    _planeTexId = _planeTexId2;
    _planeTexId2 = tmp;
  }
  if(_cam.x > _tabObjet[1].x - 3 && _cam.x < _tabObjet[1].x + 3 && _cam.z < _tabObjet[1].y)
  { 
    if (_timer_vie_dehors - _timer_vie_dehors_p > 500)
    {
      printf("Vous êtes pas dans le bon sens ^^ !\n");
      _timer_vie_dehors_p = _timer_vie_dehors;
      // if(_cheat == 0)
      //   loosepdv(15);
    }
  }
}

void ifloose ()
{
  if((_time_musique_playing <= 0 || _lifebar < -100) && _loose != 1)
  {
    Mix_PlayChannel(4, _son_cri_fin, 0);
    _loose = 1;
  }
}
/*!\brief function called by GL4Dummies' loop at draw.*/
static void draw(void) 
{
  GLfloat cam[2] = {0, 0};
  if(_loose != 1)
    ifgagne();
  ifloose();
  SDL_PumpEvents();
  /* gets the mouse coordinates on the window */
  /* clears the OpenGL color buffer and depth buffer */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  /* sets the current program shader to _pId */
  glUseProgram(_pId);
  /* loads the identity matrix in the current GL4Dummies matrix ("modelViewMatrix") */
  gl4duLoadIdentityf();
  /* modifies the current matrix to simulate camera position and orientation in the scene */
  /* see gl4duLookAtf documentation or gluLookAt documentation */
  gl4duLookAtf(_cam.x, _cam.y, _cam.z, 
       _cam.x - sin(_cam.theta), _cam.y - tan(_cam.sigma), _cam.z - cos(_cam.theta), 
       0.0, 1.0,0.0);
  //printf("cam.x : %f -- cam.y : %f -- cam.z : %f\n", _cam.x, _cam.y, _cam.z);

  /* sets the current texture stage to 0 */
  glActiveTexture(GL_TEXTURE0);
  /* tells the pId program that "myTexture" is set to stage 0 */
  glUniform1i(glGetUniformLocation(_pId, "myTexture"), 0);
  /* tells the pId program what is the value of fog */
  glUniform1i(glGetUniformLocation(_pId, "fog"), _fog);

  /* pushs (saves) the current matrix (modelViewMatrix), scales,
   * rotates, sends matrices to pId and then pops (restore) the
   * matrix */
  gl4duPushMatrix(); {
    gl4duTranslatef(_cam.x, 0.0, _cam.z);
    gl4duRotatef(-90, 1, 0, 0);
    gl4duScalef(_planeScale, _planeScale, 1);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  /* culls the back faces */
  glCullFace(GL_BACK);
  /* uses the checkboard texture */
  glBindTexture(GL_TEXTURE_2D, _planeTexId);
  /* sets in pId the uniform variable texRepeat to the plane scale */
  glUniform1f(glGetUniformLocation(_pId, "texRepeat"), _planeScale);
  /* tells pId that the sky is false */
  glUniform1i(glGetUniformLocation(_pId, "sky"), 0);
  cam[0] = _cam.x / (2.0 * _planeScale);
  cam[1] = -_cam.z / (2.0 * _planeScale);
  glUniform2fv(glGetUniformLocation(_pId, "cam"), 1, cam);
  /* draws the plane */
  gl4dgDraw(_plane);

  cam[0] = 0;
  cam[1] = 0;
  glUniform2fv(glGetUniformLocation(_pId, "cam"), 1, cam);

  /* this part means that the skydome always follow the camera position */
  gl4duPushMatrix(); {
    gl4duTranslatef(_cam.x, 1.0, _cam.z);
    gl4duScalef(_planeScale, _planeScale, _planeScale);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  /* culls the front faces (because we are in the skydome) */
  glCullFace(GL_FRONT);
  /* uses the sky texture (fractal cloud) */
  glBindTexture(GL_TEXTURE_2D, _skyTexId);
  /* texture repeat only once */
  glUniform1f(glGetUniformLocation(_pId, "texRepeat"), 1);
  /* tells pId that the sky is true (we are drawing the sky dome) */
  glUniform1i(glGetUniformLocation(_pId, "sky"), 1);
  /* draws the sky dome */
  gl4dgDraw(_sphere);

  //Placer les objets
  glEnable(GL_DEPTH_TEST);
  glUniform1i(glGetUniformLocation(_pId, "inv"), 1);

  if(_level == 0)
  {
    placeallmur(_tabObjet, _nbtabObjet);
    deplaceUnObjet(_tabObjet, _nbtabObjet);
  }
  else if(_level == 1)
  {
    if(_boolanimationfinlevel == 1)
    {
      int i;
      for (i = 0; i < _nbtabObjet2; ++i)
      {
        if(_tabObjet2[i].z >= 1)
        {
          _boolanimationfinlevel = 0;
          break;
        }
        printf("%f\n", _tabObjet2[i].z);
        _tabObjet2[i].z += 0.0004;
      }
    }
    placeallmur(_tabObjet2, _nbtabObjet2);
    deplaceUnObjet(_tabObjet2, _nbtabObjet2);
  }

  place_ennemy();
  
  //Placer arme
  //!\\ Attention changement de shader pour draw_arme()
  draw_arme();

  //-------------------------------------
  //Draw target
  glUseProgram(_pId);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glBindTexture(GL_TEXTURE_2D, _imtargetId);
  gl4duBindMatrix("projectionMatrix");
  gl4duPushMatrix(); 
  {
    gl4duLoadIdentityf();
    gl4duBindMatrix("modelViewMatrix");
    gl4duPushMatrix(); {
    gl4duLoadIdentityf();
    gl4duTranslatef(0.0, 0.2, -0.1);
    gl4duRotatef(45, 0, 0, 1);
    gl4duScalef(0.06, 0.06, 0.06);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  gl4duBindMatrix("projectionMatrix");
  } gl4duPopMatrix();
  gl4duBindMatrix("modelViewMatrix");
  gl4dgDraw(_plane);
  glEnable(GL_DEPTH_TEST);
  //------------------------------------
  //Draw flamme
  if(_tirArme == 0)
    {
      _xflamme = 0.11;
      _yflamme = 0.18;
    }
  if(_tirArme == 1)
  {
    if(_ryArme > 22)
    {
      _xflamme = 0.15;
      _yflamme = 0.4;
    }
    glUseProgram(_pId);
    glBindTexture(GL_TEXTURE_2D, _imflammeId);
    gl4duBindMatrix("projectionMatrix");
    gl4duPushMatrix(); 
    {
      gl4duLoadIdentityf();
      gl4duBindMatrix("modelViewMatrix");
      gl4duPushMatrix(); {
      gl4duLoadIdentityf();
      gl4duTranslatef(_xflamme, _yflamme, 0.80);
      gl4duRotatef(45, 0, 0, 1);
      gl4duScalef(0.2, 0.2, 0.2);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    gl4duBindMatrix("projectionMatrix");
    } gl4duPopMatrix();
    gl4duBindMatrix("modelViewMatrix");
    gl4dgDraw(_plane);
  }
  //---------------------------------
  // draw the lifebar
  gl4duBindMatrix("projectionMatrix");
  gl4duPushMatrix(); {
    gl4duLoadIdentityf();
    gl4duBindMatrix("modelViewMatrix");
    gl4duPushMatrix(); {
      gl4duLoadIdentityf();
      gl4duTranslatef(-0.9, -0.5, -0.1);
      if(_lifebar > 0)
        gl4duScalef(0.05, _lifebar * 0.0035, 1);
      else
        gl4duScalef(0.05, -_lifebar * 0.0035, 1);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    gl4duBindMatrix("projectionMatrix");
  } gl4duPopMatrix();
  gl4duBindMatrix("modelViewMatrix");
  if (_lifebar > 0)
    glBindTexture(GL_TEXTURE_2D, _lifeTexId);
  else
    glBindTexture(GL_TEXTURE_2D, _lifeTexId2);
  /* texture repeat only once */
  glUniform1f(glGetUniformLocation(_pId, "texRepeat"), 1);
  /* tells pId that the sky is false */
  gl4dgDraw(_plane);
  //-------------------------------------
  
  if(_tirArme == 0 && _rzArme >= -0.23)
  {
    // draw image on 3d model
    gl4duBindMatrix("projectionMatrix");
    gl4duPushMatrix(); {
      gl4duLoadIdentityf();
      gl4duBindMatrix("modelViewMatrix");
      gl4duPushMatrix(); {
        gl4duLoadIdentityf();
        //gl4duRotatef(0, 10, 0, 1);
        gl4duTranslatef(0.68, -0.40, 0.0);
        gl4duScalef(0.54, 0.63, 0.54);
        gl4duSendMatrices();
      } gl4duPopMatrix();
      gl4duBindMatrix("projectionMatrix");
    } gl4duPopMatrix();
    gl4duBindMatrix("modelViewMatrix");
    glBindTexture(GL_TEXTURE_2D, _imarme_pId);
    /* texture repeat only once */
    glUniform1f(glGetUniformLocation(_pId, "texRepeat"), 1);
    /* tells pId that the sky is false */
    gl4dgDraw(_plane);
  }
  //------------------------------------------
  update_munition(0);
  //------------------------------------------
  //draw flash
  if(_flash > 0 && _loose != 1)
  {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    gl4duBindMatrix("projectionMatrix");
    gl4duPushMatrix(); {
      gl4duLoadIdentityf();
      gl4duBindMatrix("modelViewMatrix");
      gl4duPushMatrix(); {
        gl4duLoadIdentityf();
        gl4duTranslatef(0, 0, 0);
        gl4duScalef(1, 1, 1);
        gl4duSendMatrices();
      } gl4duPopMatrix();
      gl4duBindMatrix("projectionMatrix");
    } gl4duPopMatrix();
    gl4duBindMatrix("modelViewMatrix");
    glBindTexture(GL_TEXTURE_2D, _lifeTexId2);
    glUniform1f(glGetUniformLocation(_pId, "texRepeat"), 1);
    gl4dgDraw(_plane);
    glEnable(GL_DEPTH_TEST);
    _flash--;
  }
  // draw the timer
  timer();
  //-------------------------------------
  // draw compass
  gl4duBindMatrix("projectionMatrix");
  gl4duPushMatrix(); {
    gl4duLoadIdentityf();
    gl4duBindMatrix("modelViewMatrix");
    gl4duPushMatrix(); {
      gl4duLoadIdentityf();
      gl4duTranslatef(-0.75, 0.7, -0.1);
      gl4duRotatef(_cam.theta * 180.0 / M_PI, 0, 0, 1);
      gl4duScalef(0.03 / 5.0, 1.0 / 5.0, 1.0 / 5.0);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    gl4duBindMatrix("projectionMatrix");
  } gl4duPopMatrix();
  gl4duBindMatrix("modelViewMatrix");
  /* disables cull facing and depth testing */
  glDisable(GL_CULL_FACE);
  /* uses the compass texture */
  glBindTexture(GL_TEXTURE_2D, _compassTexId);
  /* texture repeat only once */
  glUniform1f(glGetUniformLocation(_pId, "texRepeat"), 1);
  /* tells pId that the sky is false */
  glUniform1i(glGetUniformLocation(_pId, "sky"), 0);
  /* draws the compass */
  gl4dgDraw(_plane);
  
  if(_loose == 1)
  {
    gl4duwKeyDownFunc(keydown2);
    if(_cam.y > 0.2)
      _cam.y -= 0.001;
    if(_cam.theta <= 10)
      _cam.theta += 0.0025;
    if(_cam.sigma < 0.55)
      _cam.sigma += 0.0005;

  }
}

/*!\brief function called at exit. Frees used textures and clean-up
 * GL4Dummies.*/
static void quit(void) {
  if(_planeTexId)
    glDeleteTextures(1, &_planeTexId);
  if(_skyTexId)
    glDeleteTextures(1, &_skyTexId);
  if(_compassTexId)
    glDeleteTextures(1, &_compassTexId);
  if(_imchampibadId)
    glDeleteTextures(1, &_imchampibadId);
  if(_importeId)
    glDeleteTextures(1, &_importeId);
  if(_immurId)
    glDeleteTextures(1, &_immurId);
  if(_immurId2)
    glDeleteTextures(1, &_immurId2);
  assimpQuit();
  gl4duClean(GL4DU_ALL);
  Mix_CloseAudio();
  TTF_CloseFont(_police);
  TTF_CloseFont(_police2);
  TTF_CloseFont(_police3);
  TTF_Quit();
}
