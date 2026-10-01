#include "DxLib.h"
#include <fstream>
#include "json.hpp"
#include <vector>
#include <string>
#include "tinyxml2.h"

using json = nlohmann::json;
using namespace tinyxml2;
struct Ball{
    float x;
    float y;

    float vx;
    float vy;

    bool moving;
};
Ball ball;

enum GameState{
    TITLE,
    MENU,
    OPTIONS,
    TEST,
    TEST2,
    GAMEPLAY
};
//char Key[256];
//char OldKey[256] = { 0 };
bool showInputMenu = true;
int  course = 0;

int easyImg;
int normalImg;
int hardImg;
int easySelectImg;
int normalSelectImg;
int hardSelectImg;

int miniHumanImg; // �l
int miniFlagImg;  // ��
int miniWallImg;
int miniWaterImg;

struct StartData{
    int x, y;
};

struct GoalData{
    int x, y;
    int radius;
};

struct ObstacleData{
    std::string type;
    int x;
    int y;
    int width;
    int height;
    float angle;
};
struct CourseData{
    StartData start;
    GoalData goal;
    std::vector<ObstacleData> obstacles;
};
CourseData currentCourse;
CourseData LoadCourse(const std::string& filename) {
    CourseData course;
    
    std::ifstream file(filename);
    if (!file.is_open()){
        printfDx("�J���Ȃ�: %s\n", filename.c_str());
        return course;
    }
    json j;
    file >> j;

    course.start.x = j["start"]["x"];
    course.start.y = j["start"]["y"];

    course.goal.x = j["goal"]["x"];
    course.goal.y = j["goal"]["y"];
    course.goal.radius = j["goal"]["radius"];

    for (const auto& obj : j["obstacles"])
    {
        ObstacleData obstacle;

        obstacle.type = obj["type"];
        obstacle.x = obj["x"];
        obstacle.y = obj["y"];
        obstacle.width = obj["width"];
        obstacle.height = obj["height"];
        obstacle.angle = 0;

        course.obstacles.push_back(obstacle);
    }
    return course;
}
class Input
{
public:
    void Update(){
        memcpy(OldKey, Key, sizeof(Key));
        GetHitKeyStateAll(Key);
    }

    bool Press(int key){
        return Key[key] && !OldKey[key];
    }

    bool Down(int key){//�����Ă���Ԃ����Ə�����������
        return Key[key];
    }

    bool Release(int key){//�������u�Ԃ��擾��������
        return !Key[key] && OldKey[key];
    }

private:
    char Key[256]{};
    char OldKey[256]{};
};
Input input;
void DarkPanel() {
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
    DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    if (input.Press(KEY_INPUT_DOWN)){
        course = (course + 1) % 3;
    }

    if (input.Press(KEY_INPUT_UP)){
        course = (course + 2) % 3;
    }
    if (input.Press(KEY_INPUT_SPACE)){
        switch (course){
        case 0:
            currentCourse =
                LoadCourse("course/easy.json");
            ball.x = currentCourse.start.x;
            ball.y = currentCourse.start.y;

            ball.vx = 0;
            ball.vy = 0;
            break;

        case 1:
            currentCourse =
                LoadCourse("course/normal.json");
            ball.x = currentCourse.start.x;
            ball.y = currentCourse.start.y;

            ball.vx = 0;
            ball.vy = 0;
            break;

        case 2:
            currentCourse =
                LoadCourse("course/hard.json");
            ball.x = currentCourse.start.x;
            ball.y = currentCourse.start.y;

            ball.vx = 0;
            ball.vy = 0;
            break;
        }
        showInputMenu = false;
    }
    DrawGraph(
        870, 250,
        course == 0 ? easySelectImg : easyImg,
        TRUE
    );
    DrawGraph(
        870, 450,
        course == 1 ? normalSelectImg : normalImg,
        TRUE
    );
    DrawGraph(
        870, 650,
        course == 2 ? hardSelectImg : hardImg,
        TRUE
    );
}
void LoadAnimationXML(const char* xmlPath) {
    XMLDocument doc;
    if (doc.LoadFile(xmlPath) != XML_SUCCESS) return;

    XMLElement* atlas = doc.FirstChildElement("TextureAtlas");
    if (!atlas) return;
    XMLElement* sub = atlas->FirstChildElement("SubTexture");

    while (sub) {//sub は <SubTexture> を指すポインタ
        const char* name = sub->Attribute("name");//変数nameにXMLのname属性の文字列を入れる
        Frame f;
        sub->QueryIntAttribute("x", &f.x);//xの値をFrame(f)グループのxの項目に入れる
        sub->QueryIntAttribute("y", &f.y);//&はポインタ
        sub->QueryIntAttribute("width", &f.w);
        sub->QueryIntAttribute("height", &f.h);

        sub->QueryIntAttribute("frameX", &f.fx);
        sub->QueryIntAttribute("frameY", &f.fy);
        sub->QueryIntAttribute("frameWidth", &f.fw);
        sub->QueryIntAttribute("frameHeight", &f.fh);//push_backはvector の一番うしろにデータを追加する関数

        if (strncmp(name, "player_right ", 13) == 0) anim[RIGHT].push_back(f);
        else if (strncmp(name, "player_left ", 12) == 0) anim[LEFT].push_back(f);
        else if (strncmp(name, "player_up ", 10) == 0)   anim[UP].push_back(f);
        else if (strncmp(name, "player_down ", 12) == 0) anim[DOWN].push_back(f);

        sub = sub->NextSiblingElement("SubTexture");
    }
}
void DrawProgressMap(){
    int mapX = 1400;
    int mapY = 60;
    int mapWidth = 400;

    // �S�̐�
    DrawLine(
        mapX,
        mapY,
        mapX + mapWidth,
        mapY,
        GetColor(0, 0, 0)
    );

    float progress =
        (ball.x - currentCourse.start.x)
        / (float)(currentCourse.goal.x - currentCourse.start.x);

    // �ʉߍς�
    DrawLine(
        mapX,
        mapY,
        mapX + (int)(mapWidth * progress),
        mapY,
        GetColor(105, 105, 105)
    );
    // �X�^�[�g�n�_
    DrawCircle(
        mapX,
        mapY,
        5,
        GetColor(0, 0, 0),
        TRUE
    );
    // �l
    int playerPos =
        mapX + (int)(mapWidth * progress);

    DrawGraph(
        playerPos - 16,
        mapY - 16,
        miniHumanImg,
        TRUE
    );

    // �S�[��(��)
    DrawGraph(
        mapX + mapWidth - 16,
        mapY - 16,
        miniFlagImg,
        TRUE
    );//��Q��
    for (const auto& obs : currentCourse.obstacles)
    {
        float rate =
            (obs.x - currentCourse.start.x)
            / (float)(currentCourse.goal.x - currentCourse.start.x);

        int obsPos =
            mapX + (int)(mapWidth * rate);

        int image = -1;

        if (obs.type == "wall")
        {
            image = miniWallImg;
        }
        else if (obs.type == "water")
        {
            image = miniWaterImg;
        }
        if (image != -1)
        {
            DrawGraph(
                obsPos - 8,
                mapY - 8,
                image,
                TRUE
            );
        }
    }
}
void DrawCourse(){
    DrawCircle(
        currentCourse.goal.x,
        currentCourse.goal.y,
        currentCourse.goal.radius,
        GetColor(255, 255, 0),
        TRUE
    );

    // ��Q��
    for (const auto& obs : currentCourse.obstacles)
    {
        DrawBox(
            obs.x,
            obs.y,
            obs.x + obs.width,
            obs.y + obs.height,
            GetColor(255, 0, 0),
            TRUE
        );
    }
}
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int){
    ChangeWindowMode(TRUE);
    SetGraphMode(1920, 1080, 32);

    if (DxLib_Init() == -1){
        return -1;
    }
    int menuBG = LoadGraph("image/background0.png");
    easyImg = LoadGraph("image/course_easy.png");
    normalImg = LoadGraph("image/course_normal.png");
    hardImg = LoadGraph("image/course_hard.png");

    easySelectImg = LoadGraph("image/course_easy_select.png");
    normalSelectImg = LoadGraph("image/course_normal_select.png");
    hardSelectImg = LoadGraph("image/course_hard_select.png");

    miniHumanImg = LoadGraph("image/icon_hito.png");
    miniFlagImg = LoadGraph("image/icon_goal.png");
    miniWallImg = LoadGraph("image/icon_wall.png");
    miniWaterImg = LoadGraph("image/icon_water.png");

    //printfDx("menuBG = %d\n", menuBG);

    GameState currentState = TITLE;

    int menuIndex = 0;

    const char* menuItems[] = {
        "Start Game",
        "Options",
        "Test",
        "Test2",
        "Exit"
    };
    const char* helpText = "";
    const int menuCount = 5;
    int FrameCount = 0;
    int OldTime = GetNowCount();
    float FPS = 0;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    while (ProcessMessage() == 0)
    {
        // -----------------
        // �X�V����
        // -----------------
        input.Update();
        //GetHitKeyStateAll(Key);
        FrameCount++;

        if (GetNowCount() - OldTime >= 1000)
        {
            FPS = (float)FrameCount;

            FrameCount = 0;
            OldTime = GetNowCount();
        }


        switch (currentState){
        case TITLE:

            if (input.Press(KEY_INPUT_RETURN)) {
                currentState = MENU;
            }

            break;

        case MENU:

            if (input.Press(KEY_INPUT_DOWN)){
                menuIndex = (menuIndex + 1) % menuCount;
            }
            if (input.Press(KEY_INPUT_UP)) {
                menuIndex = (menuIndex - 1 + menuCount) % menuCount;
            }
            if (input.Press(KEY_INPUT_RETURN)){
                if (menuIndex == 0) {
                    currentState = GAMEPLAY;
                    showInputMenu = true;
                }

                else if (menuIndex == 1)
                    currentState = OPTIONS;

                else if (menuIndex == 2)
                    currentState = TEST;

                else if (menuIndex == 3)
                    currentState = TEST2;

                else if (menuIndex == 4) {
                    DxLib_End();
                    return 0;
                }
            }
            break;

        case OPTIONS:
            break;

        case TEST:
            break;

        case TEST2:
            break;

        case GAMEPLAY:
            break;
        }

        // -----------------
        // �߂鏈��
        // -----------------

        if (input.Press(KEY_INPUT_BACK))
        {
            switch (currentState)
            {
            case MENU:
                currentState = TITLE;
                break;

            case OPTIONS:
                currentState = MENU;
                break;

            case TEST:
                currentState = MENU;
                break;

            case TEST2:
                currentState = MENU;
                break;

            case GAMEPLAY:
                currentState = MENU;
                break;

            default:
                break;
            }
        }

        // -----------------
        // �`�揈��
        // -----------------
        // ������/////////////////////////////////////////////////////////////////////////
        switch (currentState)
        {
        case TITLE:
            helpText = "--";
            break;

        case MENU:
            switch (menuIndex)
            {
            case 0:
                helpText = "ENTER �ŃQ�[����ʂɐi�݂܂�";
                break;
            case 1:
                helpText = "ENTER �Őݒ��ʂɐi�݂܂�";
                break;
            case 2:
                helpText = "ENTER �Ńe�X�g��ʂɐi�݂܂�";
                break;
            case 3:
                helpText = "ENTER �Ńe�X�g2��ʂɐi�݂܂�";
                break;
            case 4:
                helpText = "ENTER �ŃQ�[�����I�����܂�";
                break;
            }
            break;

        case OPTIONS:
            helpText = "BACKSPACE �Ń��j���[�ɖ߂�܂�";
            break;

        case TEST:
            helpText = "BACKSPACE �Ń��j���[�ɖ߂�܂�";
            break;

        case TEST2:
            helpText = "BACKSPACE �Ń��j���[�ɖ߂�܂�";
            break;

        case GAMEPLAY:
            //helpText = "BACKSPACE �Ń��j���[�ɖ߂�܂�";
            break;
        }
        /////////////////////////////////////////////////////////////////////////////////////////
        char fpsText[32];

       
        ClearDrawScreen();

        switch (currentState)
        {
        case TITLE:

            DrawString(
                (1920 - GetDrawStringWidth("THROWING GOLF", strlen("THROWING GOLF"))) / 2,
                500,
                "THROWING GOLF",
                0xffffff);//GetColor(255, 255, 255));

            DrawString(
                (1920 - GetDrawStringWidth("Press ENTER", strlen("Press ENTER"))) / 2,
                550,
                "Press ENTER",
                GetColor(255, 255, 0));
            break;

        case MENU:
            DrawGraph(0, 0, menuBG, TRUE);
            for (int i = 0; i < menuCount; i++)
            {
                int color =
                    (i == menuIndex)
                    ? GetColor(255, 255, 0)
                    : GetColor(255, 255, 255);

                DrawString(
                    (1920 - GetDrawStringWidth(menuItems[i], strlen(menuItems[i]))) / 2,
                    390 + i * 50,
                    menuItems[i],
                    color);
            }
            break;

        case OPTIONS:

            DrawString(
                (1920 - GetDrawStringWidth("OPTIONS", strlen("OPTIONS"))) / 2,
                500,
                "OPTIONS",
                GetColor(255, 255, 255));
            break;

        case TEST:

            DrawString(
                (1920 - GetDrawStringWidth("TEST", strlen("TEST"))) / 2,
                500,
                "TEST",
                GetColor(255, 255, 255));
            break;

        case TEST2:

            DrawString(
                (1920 - GetDrawStringWidth("TEST2", strlen("TEST2"))) / 2,
                500,
                "TEST2",
                GetColor(255, 255, 255));

            break;

        case GAMEPLAY:
            helpText = "Back space ��Menu�ɖ߂�܂�";
            DrawGraph(0, 0, menuBG, TRUE);
            //showInputMenu = true;
            if (showInputMenu){
                DarkPanel();
            }
            //DrawString(
            //    (1920 - GetDrawStringWidth("GAMEPLAY", strlen("GAMEPLAY"))) / 2,
            //    350,
            //    "GAMEPLAY",
            //    GetColor(255, 255, 255));
            else {
                DrawProgressMap();
                //DrawCourse();
            }
            break;
        }
        DrawBox(
            0,
            1020,
            1920,
            1080,
            GetColor(30, 30, 30),
            TRUE
        );

        DrawLine(//�������̏�̔���
            0,
            1020,
            1920,
            1020,
            GetColor(255, 255, 255)
        );
        DrawString(
            20,
            1040,
            helpText,
            GetColor(255, 255, 255)
        );
        sprintf_s(
            fpsText,
            "FPS : %.0f",
            FPS
        );

        DrawString(
            10,
            10,
            fpsText,
            GetColor(255, 255, 255)
        );
        ScreenFlip();
        //memcpy(OldKey, Key, 256);

    }
    DeleteGraph(menuBG);
    DxLib_End();
    return 0;
}