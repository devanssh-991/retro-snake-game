#include <iostream>
#include <raylib.h>
#include<deque>
#include <raymath.h>
using namespace std;

Color green = {173,204,96,255};

Color darkgreen = {43,51,24,255};

int cellsize = 30;
int cellcount = 25;

Sound eatsound;
Sound wallsound;  
enum GameScreen { MENU, PLAYING, GAME_OVER };
GameScreen currentScreen = MENU;


int flag = 1;

double lastupdatetime = 0;

int offset  = 75; 
int score=0; 

bool eventtriggered(double interval){
    double currenttime = GetTime();
    if(currenttime-lastupdatetime >=interval){
        lastupdatetime = currenttime;
        return true;
    }
    return false;
}


bool elementInDeque(Vector2 element, deque<Vector2> deque){
     for(unsigned int i=0; i<deque.size(); i++){
        if(Vector2Equals(deque[i], element))
            return true;
     }
     return false;
}


class Food {
    public:
        Vector2 position;
        Texture2D texture;

        Food(deque<Vector2> snakeBody){
            Image image = LoadImage("food.png");
            texture = LoadTextureFromImage(image);
            UnloadImage(image);
            position = generateRandomPos(snakeBody, 1);

        }

        ~Food(){
            UnloadTexture(texture);
        }

        void draw(){
            DrawTexture(texture,offset+position.x*cellsize,offset+position.y*cellsize, WHITE);
        }

        Vector2 Generaterandomcell(){
            float x = GetRandomValue(0,cellcount-1);
            float y = GetRandomValue(0,cellcount-1);
            return Vector2{x,y};
        } 


        Vector2 generateRandomPos(deque<Vector2> snakeBody, int flag){
            Vector2 position;
            if(flag){
                position = {9,9};
            }
            else{            
                position = Generaterandomcell();
                while(elementInDeque(position,snakeBody)){
                    position = Generaterandomcell();
                }
            }   
            return position;
        }

};


class Snake{
    public:
        deque<Vector2> body = {Vector2{6,9},Vector2{5,9}, Vector2{4,9}};
        //screen coordinates is different from cartesian coordinates. origin is at top left corner and x increases from top left to right and y increases downwards
        Vector2 direction = {1,0};
    
        bool addSegment = false;


    void draw(){
        for(unsigned int i=0; i<body.size(); i++){
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset+x * cellsize, offset+y * cellsize, (float)cellsize, (float)cellsize};
            
            DrawRectangleRounded(segment,0.5,6,darkgreen );
        }
    }

    void update(){
        
        body.push_front(Vector2Add(body[0],direction)); 
        if(addSegment==true){
            addSegment=false;
        }
        else{
            body.pop_back();
        }

    }
    void reset(){
        body = {Vector2{6,9},Vector2{5,9},Vector2{4,9}};

    }

    
};
 
class Game{
    public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;

    Game(){
        InitAudioDevice();
        eatsound = LoadSound("sounds/eat.mp3");
        wallsound = LoadSound("sounds/wall.mp3");
    }

    ~Game(){
        UnloadSound(eatsound);
        UnloadSound(wallsound);
        CloseAudioDevice(); 
    }


    void draw(){
        food.draw();
        snake.draw();
    }
    void Update(){
        if(running){
            snake.update();
            checkCollisionsWithFood();
            checkCollisionWithEdges();
            checkcollisionwithtail();
        }
    
    }

    void checkCollisionsWithFood(){
        
        if(Vector2Equals(snake.body[0], food.position)){
        
            food.position = food.generateRandomPos(snake.body,0);
            snake.addSegment = true;
            score++;
            PlaySound(eatsound);
        }
    }

    void checkCollisionWithEdges(){
        if(snake.body[0].x== cellcount || snake.body[0].x==-1){
            GameOver();

        }

        if(snake.body[0].y==cellcount || snake.body[0].y==-1){
            GameOver();
        }
    }

    void GameOver(){
        snake.reset();
        food.position = food.generateRandomPos(snake.body,0); 
        running = false;
        score=0;
        PlaySound(wallsound);

    }

    void checkcollisionwithtail(){
        //copy of snake body
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if(elementInDeque(snake.body[0], headlessBody)){
            GameOver();
        }
    }


};



int main () {

    cout<<"Stating the game";
    
    InitWindow(2*offset+cellsize*cellcount,2*offset+cellcount*cellsize, "Retro Snake");
    SetTargetFPS(120);

    Game game = Game();
    

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(green);
        
        switch (currentScreen)
        {
        case MENU:
        
            DrawText("Retro Snake", offset + 50, offset, 50, darkgreen);
            DrawText("Press [ENTER] to Start", offset + 20, offset + 100, 30, darkgreen);
            DrawText("Press [ESC] to Exit", offset + 20, offset + 140, 20, darkgreen);
            if (IsKeyPressed(KEY_ENTER)) {
                currentScreen = PLAYING;
                game.running = true;
            }
            break;

        case PLAYING:
            ClearBackground(green);


            //change the value inside event triggered to move the snake fast or slow
            if(eventtriggered(0.1)){
                game.Update();
            }


            if((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && game.snake.direction.y != 1){
                game.snake.direction = {0,-1};
                game.running = true;
            }

            if((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && game.snake.direction.y != -1){
                game.snake.direction = {0,1};
                game.running = true;
            }
            if((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && game.snake.direction.x!= 1 ){
                game.snake.direction = {-1,0};
                game.running = true;
            }
            if((IsKeyPressed(KEY_RIGHT)|| IsKeyPressed(KEY_D)) && game.snake.direction.x !=-1){
                game.snake.direction = {1,0};
                game.running = true;
            }
            DrawRectangleLinesEx(Rectangle{(float)offset-5,(float)offset-5,(float)cellsize*cellcount+10,(float)cellsize*cellcount+10},5,darkgreen);
            DrawText("Retro Snake",offset-5,20,40,darkgreen);
            DrawText(TextFormat("%i", score),offset-5,offset+cellsize*cellcount+10,40,darkgreen);
            game.draw(); 

            if(!game.running){
                currentScreen = GAME_OVER;
            }
            break;
        
        case GAME_OVER:
                DrawText("Game Over!", offset + 70, offset, 50, darkgreen);
                DrawText(TextFormat("Final Score: %i", score), offset + 70, offset + 60, 30, darkgreen);
                DrawText("Press [M] for Menu", offset + 20, offset + 120, 20, darkgreen);

                if (IsKeyPressed(KEY_M)) {
                    currentScreen = MENU;
                }
                break;

        
        }
        

        EndDrawing();
    }


    CloseWindow();
    return 0;
}