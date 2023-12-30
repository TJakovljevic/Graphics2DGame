//4hero
//set sprajtova je kljucan u animaciji
//to se naziva spritesheet
//ukoliko idemo kroz neki red i vrtimo ga u krug imamo animaciju kretanja
//spite je neka slicica karaktera ili efekta i sl.
#include <main_state.h>
#include <glad/glad.h>
#include <math.h>




#include <rafgl.h>

#include <game_constants.h>

static rafgl_raster_t doge;
static rafgl_raster_t upscaled_doge;
static rafgl_raster_t raster, raster2, raster3;
static rafgl_raster_t checker;

static rafgl_texture_t texture;

static rafgl_spritesheet_t hero;

static rafgl_spritesheet_t chest;

static rafgl_spritesheet_t pokemon;

static rafgl_spritesheet_t pokeball;



#define NUMBER_OF_TILES 17
#define MAX_PARTICLES 500
rafgl_raster_t tiles[NUMBER_OF_TILES];

//velicina matrice, koliki da nam bude svet i ta matrica
#define WORLD_SIZE 128
int tile_world[WORLD_SIZE][WORLD_SIZE];

#define TILE_SIZE 64


static int raster_width = RASTER_WIDTH, raster_height = RASTER_HEIGHT;

static char save_file[256];
int save_file_no = 0;

typedef struct _particle_t
{
    float x, y, dx, dy;
    int life;


} particle_t;
particle_t particles[MAX_PARTICLES];



void draw_particles(rafgl_raster_t *raster)
{
    int i;
    particle_t p;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        p = particles[i];
        if(p.life <= 0) continue;
        //funkcijom draw line iscrtavamo liniju (uradjeno preko bresenhamnov alg. za iscrtavanje duzi)
        //
        //kako radi: na nekom rasteru od tacke x0, y0, do x1, y1, isrtaj duz u boji toj i toj
        //x0 i y0 se racunaju tako sto od trenutne pozicije oduzmemo brzinu kojom se krecemo, dakle to nam je otprilike prethodna pozicija
        int randomValue = randf()*3 + 1;
        switch(randomValue){
        case 1: {
             rafgl_raster_draw_line(raster, p.x - p.dx, p.y - p.dy, p.x, p.y, rafgl_RGB(255,255,0));
            break;
        }
        case 2:{
         rafgl_raster_draw_line(raster, p.x - p.dx, p.y - p.dy, p.x, p.y, rafgl_RGB(255,0,0));
         break;
        }

        }

    }
}

static float elasticity = 0.6;

void update_particles(float delta_time)
{
    int i;
    for(i = 0; i < MAX_PARTICLES; i++)
    {
        if(particles[i].life <= 0) continue;

        particles[i].life--;

        particles[i].x += particles[i].dx;
        particles[i].y += particles[i].dy;
        particles[i].dx *= 0.995f;
        particles[i].dy *= 0.995f;
        particles[i].dy += 0.05;

        //ukoliko smo udarili u levi zid
        //onda pomeramo da bude x na 0
        //i samo promenimo smer za dx
        //to smo mogli i bez abs samo sa minusom
        if(particles[i].x < 0)
        {
            particles[i].x = 0;
            //elasticity nam je gubitak u svakom slucaju, ali imamo i neki random gubitak
            //da se ne bi svi partikli odbijali istom jacinom od zida
            particles[i].dx = (rafgl_abs_m(particles[i].dx)) * randf() * elasticity;
        }

        //analogno za y
        //odnosno "udaranje o plafon"
        if(particles[i].y < 0)
        {
            particles[i].y = 0;
            particles[i].dy = (rafgl_abs_m(particles[i].dy)) * randf() * elasticity;
        }

        //udaranje u desni zid
        //minus elasticity zbog obrtanja smera
        if(particles[i].x >= raster_width)
        {
            particles[i].x = raster_width - 1;
            particles[i].dx = (rafgl_abs_m(particles[i].dx)) * randf() * (-elasticity);
        }

        //analogno za "udaranje u pod"
        if(particles[i].y >= raster_height)
        {
            particles[i].y = raster_height - 1;
            particles[i].dy = (rafgl_abs_m(particles[i].dy)) * randf() * (-elasticity);
        }

    }

}

typedef struct _star_t
{
    int x, y, z;
} star_t;

#define STAR_MAX 500
#define STAR_Z_MAX 2000
star_t stars[STAR_MAX];

void init_stars(void)
{
    int i;
    for(i = 0; i < STAR_MAX; i++)
    {
        //z je od 0 do 2000
        //x i y mogu da budu od -1000 do +1000
        stars[i].z = randf() * STAR_Z_MAX;
        stars[i].x = randf() * STAR_Z_MAX - STAR_Z_MAX / 2;
        stars[i].y = randf() * STAR_Z_MAX - STAR_Z_MAX / 2;
    }
}

void update_stars(int speed)
{
    int i;

    for(i = 0; i < STAR_MAX; i++)
    {
        stars[i].z -= speed;
        if(stars[i].z <= 1)
        {
            stars[i].z += STAR_Z_MAX;
            stars[i].x = randf() * STAR_Z_MAX - STAR_Z_MAX / 2;
            stars[i].y = randf() * STAR_Z_MAX - STAR_Z_MAX / 2;
        }
    }
}
int hero_pos_x = WORLD_SIZE / 2;
int hero_pos_y = WORLD_SIZE/ 2;
int direction = 0;
void render_stars1(rafgl_raster_t *raster, int speed)
{
    int i, brightness;
    int arrowSize = 20;
    float sx0, sy0, sx1, sy1;
    double angle = 0;

   for (i = 0; i < STAR_MAX; i++)
    {
        if (direction == 0)
        {
            sx1 = hero_pos_x + 30;
            sy1 = hero_pos_y + 200;

            sx0 = hero_pos_x + 30;
            sy0 = hero_pos_y;

            angle = atan2f(sy1 - sy0, sx1 - sx0);
        }
        else if (direction == 1)
        {
            sx1 = hero_pos_x - 150;
            sy1 = hero_pos_y + 50;

            sx0 = hero_pos_x;
            sy0 = hero_pos_y + 50;

            angle = atan2f(sy1 - sy0, sx1 - sx0);
        }
        else if (direction == 2)
        {
            sx1 = hero_pos_x + 200;
            sy1 = hero_pos_y + 50;

            sx0 = hero_pos_x;
            sy0 = hero_pos_y + 50;

            angle = atan2f(sy1 - sy0, sx1 - sx0);
        }
        else if (direction == 3)
        {
            sx1 = hero_pos_x + 30;
            sy1 = hero_pos_y - 200;

            sx0 = hero_pos_x + 30;
            sy0 = hero_pos_y;

            angle = atan2f(sy1 - sy0, sx1 - sx0);
        }

        brightness = 255 - ((float)stars[i].z / STAR_Z_MAX) * 255.0f;

        // LINIJA
        rafgl_raster_draw_line(raster, sx1, sy1, sx0, sy0, rafgl_RGB(brightness, brightness, brightness));

        // GORNJA STRELA
        double arrowXUpper = sx1 - arrowSize * cos(angle - M_PI / 6);
        double arrowYUpper = sy1 - arrowSize * sin(angle - M_PI / 6);
        rafgl_raster_draw_line(raster, sx1, sy1, arrowXUpper, arrowYUpper, rafgl_RGB(brightness, brightness, brightness));

        // DONJA STRELA
        double arrowXLower = sx1 - arrowSize * cos(angle + M_PI / 6);
        double arrowYLower = sy1 - arrowSize * sin(angle + M_PI / 6);
        rafgl_raster_draw_line(raster, sx1, sy1, arrowXLower, arrowYLower, rafgl_RGB(brightness, brightness, brightness));
    }
}

void render_stars2(rafgl_raster_t *raster, int speed)
{
     int brightness;
    float sx, sy;
    float diagonalOffsetX = 50.0f;
int frameCounter = 0;
    for (sx = 0; sx <  raster_width; sx++)
    {
          frameCounter++;


        if (frameCounter % 10 == 0){

        sy = 0;


        sy += randf() * raster_height;


        if (sy > raster_height)
        {
            sy = 0;
        }


        rafgl_raster_draw_line(raster, sx, sy, sx, sy + 10, rafgl_RGB(255,255,255));
        }

    }
}

void render_stars3(rafgl_raster_t *raster, int speed){





}

float location = 0;
float selector = 0;
int camx = 600, camy = 600;
int selected_x, selected_y;

//flag da li se krecemo ili stojimo
int animation_running = 0;
//koji je indeks slike po x osi iz spritesheeta, tj koju kolonu gledamo
int animation_frame = 0;
//koji je indeks slike po y osi iz spritesheeta, tj koji red gledamo


//pozicija karaktera na pocetku

//brzina kretanja karaktera


int camera_speed = 3;
int hero_speed = 150;
//da se ne menja u svakom frameu, da ne bismo prebrzo isli kroz animaciju
//vec kazemo koliko frameova da traje neka animacija, tj jedna slicica
int hover_frames = 0;


int chest_HP = 1;
int chest_pos_x;
int chest_pos_y;
int chest_animation = 0;


int pokemon_pos_x = 200;
int pokemon_pos_y = 240;
int pokemon_flag = 0;

int pokeball_pos_x = 0;
int pokeball_pos_y = 0;
int pokeball_speed = 4;
int pokeball_ready = 0;

#define MAXBallThrow 200


void init_tilemap(void)
{

    int x, y;


    //prolazimo kroz svet i randomujemo
    //i imamo 20% ssanse za generisanje drveca i 80%sanse da generisemo travu
    //
    for(y = 0; y < WORLD_SIZE; y++)
    {
        for(x = 0; x < WORLD_SIZE; x++)
        {


            if(y == chest_pos_y/TILE_SIZE + 5 && x == chest_pos_x/TILE_SIZE){
                tile_world[y][x] = 16;
            }

            else{
                 if(randf() <0.2f)
            {
                //generisanje drveca, tajlovi sa indeksima 3,4,5
                tile_world[y][x] = 3 + rand() % 3;
            }
            else
            {
                //generisanje trave, tajlovi sa indeksima 0,1,2
                tile_world[y][x] = rand() % 3;
            }

            }
        }

    }
}



void render_tilemap(rafgl_raster_t *raster)
{
    int x, y;
    int x0 = camx / TILE_SIZE;
    //koliko nam tajlova staje na ekran tajlovano po x koordinati
    //tj ako nam je ekran sirok 640 piksela, u njega staje 10 tajlova, a ovo +1 dodajemo
    //u slucaju kada se nalazimo na nekoj ivici, pa nam se prikazuje po pola tajla
    //i za y imamo +2, jer ima onih drveca koji imaju viska gore
    //pa da ne bismo to odsekli
    int x1 = x0 + (raster_width / TILE_SIZE) + 1;
    int y0 = camy / TILE_SIZE;
    int y1 = y0 + (raster_height / TILE_SIZE) + 2;

    //klempujemo
    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;

    //klempujemo
    if(x0 >= WORLD_SIZE) x0 = WORLD_SIZE - 1;
    if(y0 >= WORLD_SIZE) y0 = WORLD_SIZE - 1;
    if(x1 >= WORLD_SIZE) x1 = WORLD_SIZE - 1;
    if(y1 >= WORLD_SIZE) y1 = WORLD_SIZE - 1;

    //pointer ka tipu tajla kojeg trenutno iscrtavamo
    //inace tajlovi su nam ucitani dole u for petlji u inicijalizaciji
    rafgl_raster_t *draw_tile;

    //prolazimo sa y i x od y0 do y1 i od x0 do x1, to su svi tajlovi koji se vide
    //tj svi tajlovi koje treba da iscrtamo
    //kako izvlacimo koji je to tip tajla, tako sto uzimamo tile_world sa tom pozicijom i to modujemo sa brojem tajlova
    //i zatim to dodoao na niz tiles, tj pokazivac na pocetak niza
    // ovo je kao da smo uradili &tiles[tile_world[y][x] % NUMBER_OF_TILES)]
    //zatim pozivamo funkciju za iscrtavanje rastera i njoj prosledjujemo na koji raster, kog tipa je tajl i
    //na kojoj poziciji
    //x*tile_size(tj to je pozicija tog tajla u svetu), ali mi ga zelimo u odnosu na pocetak ekrana, tako da zato oduzimamo i camx
    //
    //u vecini slucajeva ce nam ovo (- draw_tile->height + TILE_SIZE) biti 0, tj to nece raditi nista
    // ali u slucaju da je taj tajl malo veci po y, njega cemo samo pomeriti malo gore
    //
    //nama je bitan redosled iscrtavanja tajlova, tako da moramo prvo da iscrtamo sve gornje, pa tek onda sve donje tajlove
    //jer donji moze da sadrzi ono sto se crta preko onog iznad
    //da radimo suprotno, ne bismo videli vrhove jelki, jer bi one bile ispod tajlova koji su se preslikali preko njih
    //
    //a ukoliko kod y ne bismo stavili ovaj offset, tajl bi krenuo da se iscrtava odakle bi trebao inace, tako da bismo imali vrhove i providan deo
    //dok bi donji deo jelkica bio presecen, jer se nista ne bi iscrtavalo nakon sto se iscrta prvih 64piksela po y
    //
    for(y = y0; y <= y1; y++)
    {
        for(x = x0; x <= x1; x++)
        {


            draw_tile = tiles + (tile_world[y][x] % NUMBER_OF_TILES);
            rafgl_raster_draw_raster(raster, draw_tile, x * TILE_SIZE - camx, y * TILE_SIZE - camy - draw_tile->height + TILE_SIZE);
        }
    }

    rafgl_raster_draw_rectangle(raster, selected_x * TILE_SIZE - camx, selected_y * TILE_SIZE - camy, TILE_SIZE, TILE_SIZE, rafgl_RGB(255, 255, 0));



}

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    /* inicijalizacija */
    /* raster init nam nije potreban ako radimo load from image */
    rafgl_raster_load_from_image(&doge, "res/images/doge.png");
    rafgl_raster_load_from_image(&checker, "res/images/checker32.png");

    raster_width = width;
    raster_height = height;

    rafgl_raster_init(&upscaled_doge, raster_width, raster_height);
    rafgl_raster_bilinear_upsample(&upscaled_doge, &doge);


    rafgl_raster_init(&raster, raster_width, raster_height);
    rafgl_raster_init(&raster2, raster_width, raster_height);


    //spritesheet je inicijalizovan tako sto prosledimo
    //koju sliku ucitavamo
    //i koliko ima slicica po x, a koliko po y
    //i to smo samo zapamtili u sprite sheet hero
    init_tilemap();
    init_stars();
    rafgl_spritesheet_init(&chest, "res/images/chest21.jpg", 2, 1);
    //rafgl_spritesheet_init(&hero, "res/images/character.png", 10, 4);
    rafgl_spritesheet_init(&hero, "res/images/RED-export.png", 4, 4);
    rafgl_spritesheet_init(&pokemon, "res/images/1-b-n-export.png", 4, 4);
    rafgl_spritesheet_init(&pokeball, "res/images/pokeball-export1.png",1,1);

    //rafgl_spritesheet_init(&pokemon, "res/images/emboar.png",6, 4);


    chest_pos_x = rand() % 500 + 1;
    chest_pos_y = rand() % 500 + 1;

    pokeball_pos_x = rand() % 500 + 1;
    pokeball_pos_y = rand() % 500 + 1;

    pokemon_pos_x = rand() % 500 + 1;
    pokeball_pos_y =rand() % 500 + 1;


    int i;

    //naziv putanje do tajla kog zelimo da ucitamo
    char tile_path[256];

    //prolazimo kroz sve tajlove i ucitavamo slike u rastere za odgovarajuce pozicije u nizu
    for(i = 0; i < NUMBER_OF_TILES; i++)
    {
        //ovde ce see samo zameniti ovo %d sa trenutnim indeksom u foru
        //tako da cemo u tiles niz imati ucitane sve slike redosledom od 0 do16
        sprintf(tile_path, "res/tiles/svgset%d.png", i);
        rafgl_raster_load_from_image(&tiles[i], tile_path);
    }

    //i sada kada imamo ucitane tajloce mozemo i da inicijalizujemo tilemapu
    init_tilemap();
    rafgl_texture_init(&texture);
}




void drawComicCloud(rafgl_raster_t *raster, int centerX, int centerY, int numCircles) {

    rafgl_raster_draw_circle(raster, centerX - 20, centerY, 15, rafgl_RGB(255, 255, 255));
    rafgl_raster_draw_circle(raster, centerX, centerY, 25, rafgl_RGB(255, 255, 255));
    rafgl_raster_draw_circle(raster, centerX + 20, centerY, 20, rafgl_RGB(255, 255, 255));
    rafgl_raster_draw_circle(raster, centerX + 40, centerY, 15, rafgl_RGB(255, 255, 255));
    rafgl_raster_draw_circle(raster, centerX + 15, centerY - 15, 20, rafgl_RGB(255, 255, 255));
    rafgl_raster_draw_circle(raster, centerX + 35, centerY - 15, 15, rafgl_RGB(255, 255, 255));
}
void checkHP(){

    if(hero_pos_x/TILE_SIZE == chest_pos_x/TILE_SIZE && hero_pos_y/TILE_SIZE==chest_pos_y/TILE_SIZE ){
        printf("Nadjen: %d %d\n", chest_pos_x/TILE_SIZE, chest_pos_y/TILE_SIZE);
        chest_HP = 0;
        chest_animation = 1;
        rafgl_raster_draw_string(&raster, "You Won", hero_pos_x-80, hero_pos_y-100,rafgl_RGB(168,0,40), 32);
        draw_particles(&raster);

    }else{
        chest_HP = 1;
        chest_animation = 0;
        //VECI
         rafgl_raster_draw_circle(&raster, hero_pos_x - 7, hero_pos_y - 10, 10, rafgl_RGB(255,255,255));
        //MANJI
         rafgl_raster_draw_circle(&raster, hero_pos_x + 10, hero_pos_y + 10 , 5, rafgl_RGB(255,255,255));


        drawComicCloud(&raster, hero_pos_x - 40, hero_pos_y - 20,1);

    }

}
int carFlag = 0;
void checkPokemon(){
    int counter = 0;
    if(pokeball_pos_x/TILE_SIZE == pokemon_pos_x/TILE_SIZE && pokeball_pos_y/TILE_SIZE == pokemon_pos_y/TILE_SIZE){
        pokemon_flag = 1;
        pokemon_pos_x = 0;
        pokemon_pos_y = 0;

    }

}

void checkPokeball(){
     if(hero_pos_x/TILE_SIZE == pokeball_pos_x/TILE_SIZE && hero_pos_y/TILE_SIZE == pokeball_pos_y/TILE_SIZE && pokeball_ready == 0){
        pokeball_ready = 1;
        pokeball_pos_x = 0;
        pokeball_pos_y = 0;
        carFlag = 1;
    }

}
int rainFlag = 0;
int lightningFlag = 1;
int cameraLock = 1;
int tempX;
int tempY;

int counter = 0;

int pressed;

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{


    int i, gen = 5, radius = 10;
    float angle, speed;
    int star_speed = 10 + 90 * selector;


    //ako je pritisnut levi klik
    //prodjemo koliko zelimo da generisemo partikla za svaki frame,
    //tj koliko se drzi mis, da se generise odredjen broj partikla
    //kod nas je npr gen=5
    if(chest_HP == 0)
    {
        //dokle god nam je i manje od max_particles i dokle god imamo nesto sto generisemo za taj frame ulazimo u for
        for(i = 0; (i < MAX_PARTICLES) && gen; i++)
        {
            //ako se partikal ne koristi, onda ga "ozivljavamo"
            //za zivot dobijemo nasumican broj izmedju 100 i 200
            //i kao pocetnu poziciju mu stavimo mesto gde je kliknuto

            if(particles[i].life <= 0)
            {
                particles[i].life = 100 * randf() + 100;
                particles[i].x = chest_pos_x + 10;
                particles[i].y = chest_pos_y + 40;

                //ovde generisemo nasumicni ugao kretanja, u radijanima
                //dakle imamo neki broj od 0 do 1 puta 2pi
                //nasumicna brzina, gde nam je radius pocetna brzina
                //koju mnozimo sa random oposegom od 0.3 do 0.7
                //i dx i dy namestimo da bude cos ili sinus ugla puta brzina
                //
                //na taj nacin smo izgenerisali brzinu kruzno oko centra
                //i samnjimo gen, koji nam je jedan od uslova za ulazak u ovu petlju
                angle = randf() * M_PI *  2.0f;
                speed = ( 0.3f + 0.7 * randf()) * radius;
                particles[i].dx = cosf(angle) * speed;
                particles[i].dy = sinf(angle) * speed;
                gen--;

            }
        }


    }

    update_particles(delta_time);
    if(pokeball_ready == 1){
        update_stars(star_speed);
    }








    /* hendluj input */
    if(game_data->is_lmb_down && game_data->is_rmb_down)
    {
        pressed = 1;
        location = rafgl_clampf(game_data->mouse_pos_y, 0, raster_height - 1);
        selector = 1.0f * location / raster_height;
    }
    else
    {
        pressed = 0;
    }


    selected_x = rafgl_clampi((game_data->mouse_pos_x + camx) / TILE_SIZE, 0, WORLD_SIZE - 1);
    selected_y = rafgl_clampi((game_data->mouse_pos_y + camy) / TILE_SIZE, 0, WORLD_SIZE - 1);

    /* izmeni raster */

    int x, y;

    float xn, yn;

    rafgl_pixel_rgb_t samp, sampled, sampled2, resulting, resulting2, result;

      int cx = raster_width/2, cy = raster_height/2;
    //poluprecnik kruga
    int r = 450;

    //distanca piksela od centra
    //i koeficijent za jacinu efekta
    float dist, vignette_factor = 1.5;


    //pretpostavimo da se krecemo
    animation_running = 1;

    //direkcija nam predstavlja koji red iz spritesheeta treba da uzmemo, tj to nam je kao y iz spitesheeta
    //broj koji dodeljujemo directionu, zavisi od organizacije pravaca, tj redova u okviru spritesheeta

    //ako je duplo w pritisnuto, onda menjamo poziciju heroa tako sto ga pomeramo ka gore i stavimo da je njegova direkcija 2




    //ZAKOMENTARISAN KOD je za follow spritove pokemona
    checkPokemon();
    checkPokeball();

    if(cameraLock == 1){

        if(game_data->keys_down[RAFGL_KEY_W])
        {

                camy -= camera_speed;
                chest_pos_y += camera_speed;
                if(pokeball_ready == 0){
                    pokeball_pos_y += camera_speed;
                }
                if(pokemon_flag == 1){
                    pokemon_pos_y = hero_pos_y + 40;
                    pokemon_pos_x = hero_pos_x;
                }else pokemon_pos_y += camera_speed;
                hero_pos_y = hero_pos_y - hero_speed * delta_time;
                direction = 3;


        }
         //ako je s pritisnuto, onda menjamo poziciju heroa tako sto ga pomeramo ka dole i stavimo da je njegova direkcija 0
        else if(game_data->keys_down[RAFGL_KEY_S])
        {

                camy += camera_speed;
                chest_pos_y-=camera_speed;
                  if(pokeball_ready == 0){
                    pokeball_pos_y -= camera_speed;
                }
                if(pokemon_flag == 1){
                    pokemon_pos_y = hero_pos_y - 40;
                    pokemon_pos_x = hero_pos_x;
                }else pokemon_pos_y -= camera_speed;
                hero_pos_y = hero_pos_y + hero_speed * delta_time;
                direction = 0;


        }
        //ako je s pritisnuto, onda menjamo poziciju heroa tako sto ga pomeramo ka levo i stavimo da je njegova direkcija 1
        else if(game_data->keys_down[RAFGL_KEY_A])
        {

                camx -= camera_speed;
                chest_pos_x +=camera_speed;
                  if(pokeball_ready == 0){
                    pokeball_pos_x += camera_speed;
                }
                if(pokemon_flag == 1){
                    pokemon_pos_x = hero_pos_x + 40;
                    pokemon_pos_y = hero_pos_y;
                }else pokemon_pos_x += camera_speed;
                hero_pos_x = hero_pos_x - hero_speed * delta_time;
                direction = 1;


        }
        //ako je s pritisnuto, onda menjamo poziciju heroa tako sto ga pomeramo ka desno i stavimo da je njegova direkcija 3
        else if(game_data->keys_down[RAFGL_KEY_D])
        {

                camx += camera_speed;
                chest_pos_x -= camera_speed;
                 if(pokeball_ready == 0){
                    pokeball_pos_x -= camera_speed;
                }
                //Distance between trainer and pokeball

                if(pokemon_flag == 1){
                    pokemon_pos_x = hero_pos_x - 40;
                    pokemon_pos_y = hero_pos_y;
                }else pokemon_pos_x -= camera_speed;
                hero_pos_x = hero_pos_x + hero_speed * delta_time;
                direction = 2;


        }

        else
        {
            //ipak se ne krecemo
            animation_running = 0;
        }

        //ako zelimo da animiramo, tj kliknuto je nesto od wasd, dakle krecemo se, pa zato i hocemo da promenimo polozaj
        if(animation_running)
        {
            //hover frames je koliko frameova zelimo da stojimo na jednom spriteu, tj svaki sprite ce se zadrzati po 5 frameova
            //i tek kad hover padne na 0 mi cemo preci na sledecu animaciju
            if(hover_frames == 0)
            {
                //i samo kazemo ovo, tj kazemo da se kroz ove animacije krecemo na desno
                //i ako smo dosli do kraja, da se vratimo na pocetku tog reda
                //tj animation frame nam je x pozicija u jednom redu dokle smo stigli, kao x iz spritesheeta
                animation_frame = (animation_frame + 1) % 4;
                //kada bismo ovde stavili 1, menjali bismo animaciju kretanja u svakom frameu, pa bi nam bilo prebrzo
                hover_frames = 5;
            }
            else
            {
                //smanjujemo hover dok ne dodjemo do 0
                hover_frames--;
            }

        }

        }

    /* izmeni raster */







    for(y = 0; y < raster_height; y++)
    {
        yn = 1.0f * y / raster_height;
        for(x = 0; x < raster_width; x++)
        {
            xn = 1.0f * x / raster_width;

            sampled = pixel_at_m(upscaled_doge, x, y);
            sampled2 = rafgl_point_sample(&doge, xn, yn);

            resulting = sampled;
            resulting2 = sampled2;





            pixel_at_m(raster, x, y) = result;

           //pixel_at_m(raster, x, y) = result;

            pixel_at_m(raster, x, y) = resulting;
            pixel_at_m(raster2, x, y) = resulting2;


            if(pressed && rafgl_distance1D(location, y) < 3 && x > raster_width - 15)
            {
                pixel_at_m(raster, x, y).rgba = rafgl_RGB(255, 0, 0);
            }

        }
    }

    //pomocna funkcija koja kaze na koji raster da ucrta spritesheet
    //a spritesheet ce u initu biti veoma jednostavno definisan
    //kada zelimo da iscrtamo sprite hero, samo cemo staviti u koji raster iscrtavamo
    //koji spritesheet, koji x frame, koji y frame
    //i na kojoj poziciji unutar tog rastera zelimo da iscrtamo


    //Ovaj deo je za pomeranje zute kocke van ekrana


        if(game_data->mouse_pos_x < raster_width / 10)
        {
             if(cameraLock == 0){
                camx -= camera_speed;
                hero_pos_x += camera_speed;
                chest_pos_x += camera_speed;
                pokemon_pos_x += camera_speed;
                if(pokeball_ready == 0){
                    pokeball_pos_x += camera_speed;
                }
                hero_speed=0;
             }


        }

        //pomeranje kamere ako je pozicija misa u poslednjem desetom delu portviewa po x osi
        if(game_data->mouse_pos_x > raster_width - raster_width / 10)
        {
            if(cameraLock == 0){
                camx +=camera_speed;
                hero_pos_x -= camera_speed;

                chest_pos_x -= camera_speed;
                 pokemon_pos_x -= camera_speed;
                 if(pokeball_ready == 0){
                    pokeball_pos_x -= camera_speed;
                }
                hero_speed=0;

             }


        }

    //analogno kao i za x
        if(game_data->mouse_pos_y < raster_height / 10)
        {
            if(cameraLock == 0){
                camy -=camera_speed;
                hero_pos_y+=camera_speed;
                chest_pos_y+=camera_speed;
                pokemon_pos_y += camera_speed;
                if(pokeball_ready == 0){
                    pokeball_pos_y += camera_speed;
                }
                hero_speed=0;
             }


        }

        if(game_data->mouse_pos_y > raster_height - raster_height / 10)
        {
            if(cameraLock == 0){
                camy +=camera_speed;
                hero_pos_y-=camera_speed;
                chest_pos_y -= camera_speed;
                pokemon_pos_y -= camera_speed;
                if(pokeball_ready == 0){
                    pokeball_pos_y -= camera_speed;
                }
                hero_speed=0;
             }


        }





    if(game_data->keys_pressed[RAFGL_KEY_Y]){
        if(cameraLock == 1){
            cameraLock = 0;
        }else{
        cameraLock = 1;
        }
    }

    //kada se pritisne plus da se samo menja tajl koji je selektovan
    //menjamo brojeve u matrici tile_world od kojih nam zavisi koji tajl cemo da setujemo na tu poziciju
    if(game_data->keys_pressed[RAFGL_KEY_KP_ADD])
    {
        tile_world[selected_y][selected_x]++;
        tile_world[selected_y][selected_x] %= NUMBER_OF_TILES;
    }

    if(game_data->keys_pressed[RAFGL_KEY_KP_SUBTRACT]){

        tile_world[selected_y][selected_x]--;
        if(tile_world[selected_y][selected_x] <= 0){
            tile_world[selected_y][selected_x] += NUMBER_OF_TILES;
        }
    }

    if(game_data->keys_pressed[RAFGL_KEY_C]){
        camera_speed++;
        camera_speed%=10;

    }else if(game_data->keys_pressed[RAFGL_KEY_V]){
        camera_speed--;
        if(camera_speed <= 0){
            camera_speed = 10;
        }
    }

    if(game_data->keys_down[RAFGL_KEY_Z]){
        pokeball_pos_y = hero_pos_y - 40;
        pokeball_pos_x = hero_pos_x;
        counter = 0;
    }

    if(game_data->keys_pressed[RAFGL_KEY_R]){
       rainFlag = !rainFlag;
    }

     if(game_data->keys_pressed[RAFGL_KEY_L]){
        lightningFlag != lightningFlag;
    }



    if(game_data->keys_pressed[RAFGL_KEY_1]){
        pokeball_pos_y = 0;
        pokeball_pos_x = 0;
        pokeball_ready = 1;
    }


    switch(direction){

        case 0:{
         if(game_data->keys_down[RAFGL_KEY_X] && pokeball_ready == 1){
            printf("%d\n", counter);
            if(MAXBallThrow>= counter){
                counter+=pokeball_speed;
                pokeball_pos_y += pokeball_speed;
                pokeball_pos_x = hero_pos_x;
              }else pokeball_ready = 0;
            }
            break;
        }
        case 1:{
        if(game_data->keys_down[RAFGL_KEY_X] && pokeball_ready == 1){
            printf("%d\n", counter);
            if(MAXBallThrow >= counter){
                counter+=pokeball_speed;
                pokeball_pos_y = hero_pos_y;
                pokeball_pos_x-=pokeball_speed;
            }
            else pokeball_ready = 0;
            break;
        }
        }
        case 2:{
         if(game_data->keys_down[RAFGL_KEY_X] && pokeball_ready == 1){
           printf("%d\n", counter);
            if(MAXBallThrow >= counter){
                counter+=pokeball_speed;
                pokeball_pos_y = hero_pos_y;
                pokeball_pos_x+=pokeball_speed;
              }else pokeball_ready = 0;
            }
            break;
        }
        case 3:{
         if(game_data->keys_down[RAFGL_KEY_X] && pokeball_ready == 1){
            printf("%d\n", counter);
            if(MAXBallThrow >= counter){
                counter+=pokeball_speed;
                pokeball_pos_y -= pokeball_speed;
                pokeball_pos_x = hero_pos_x;
              }else pokeball_ready = 0;
            }
            break;
        }

        break;
        }


     render_tilemap(&raster);
     if(pokeball_ready == 1){
       render_stars1(&raster, star_speed);
     }
     checkHP();
     if(rainFlag == 1){
        render_stars2(&raster, star_speed);
     }
     if(lightningFlag == 1){
        render_stars3(&raster, star_speed);
     }
     rafgl_raster_draw_spritesheet(&raster, &hero, animation_frame, direction, hero_pos_x, hero_pos_y);
     rafgl_raster_draw_spritesheet(&raster, &chest, chest_animation,0,chest_pos_x, chest_pos_y);
     if(pokemon_flag == 1){
      rafgl_raster_draw_spritesheet(&raster, &pokemon, animation_frame, direction, pokemon_pos_x, pokemon_pos_y);
     }else rafgl_raster_draw_spritesheet(&raster, &pokemon, 0,0, pokemon_pos_x, pokemon_pos_y);

     rafgl_raster_draw_spritesheet(&raster, &pokeball,0,0, pokeball_pos_x, pokeball_pos_y);
    /* shift + s snima raster */
     if(game_data->keys_pressed[RAFGL_KEY_S] && game_data->keys_down[RAFGL_KEY_LEFT_SHIFT])
    {
        sprintf(save_file, "save%d.png", save_file_no++);
        rafgl_raster_save_to_png(&raster, save_file);
    }


}


void main_state_render(GLFWwindow *window, void *args)
{
    /* prikazi teksturu */
    rafgl_texture_load_from_raster(&texture, &raster);
    rafgl_texture_show(&texture, 0);
}


void main_state_cleanup(GLFWwindow *window, void *args)
{
    rafgl_raster_cleanup(&raster);
    rafgl_raster_cleanup(&raster2);
    rafgl_texture_cleanup(&texture);

}


