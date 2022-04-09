#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <list>
#include <algorithm>
#include <vector>
#include <functional>
#include <deque>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
float Gcost_m = 1;
float Hcost_m = 1;
int i = 0;

class Timer
{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_timepoint;

public:
    Timer()
    {
       start_timepoint = std::chrono::high_resolution_clock::now();
    }

    //~Timer()
    //{
    //    Stop();
    //}

    long long Stop()
    {
        auto end_time_point = std::chrono::high_resolution_clock::now();

        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(start_timepoint).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time_point).time_since_epoch().count();

        long long duration = end - start;
        return duration;
    }
};


struct node
{
    int x;
    int y;
    float hcost;
    float gcost;
    float fcost;
    bool traversable;
    bool closed;
    node* parent;
    std::vector<node*> vNeighbours;
};

bool CompareNodes(const node* a, const node* b)
{
    if (a->fcost != b->fcost)
        return a->fcost < b->fcost;
    else
        return a->hcost < b->hcost;
}

float Calculate_H_Cost(node* a, node* b)
{
    int D = 12;
    int D2 = 16;
    int dx = abs(a->x - b->x);
    int dy = abs(a->y - b->y);
    return 0;
    //return (D * (dx + dy) + (D2 - 2 * D) * min(dx, dy));
}

float Calculate_G_Cost(node* a, node* b)
{
    int D = 10;
    int D2 = 14;
    int dx = abs(a->x - b->x);
    int dy = abs(a->y - b->y);
    return (D * (dx + dy) + (D2 - 2 * D) * min(dx, dy));
}

bool isInSet(node* a, std::deque<node*> set)
{
    if (set.empty())
        return false;

    for (auto test : set)
    {
        if (test == a)
            return true;
    }
    return false;
}

class engine : public olc::PixelGameEngine
{
private:
    bool boardChanged;
    bool runSetup;
    int BoardWidth;
    int BoardHeight;
    int BlockSize;
    int index_start;
    int index_target;
    int* board;
    float duration;
    node* graph;
    node* start;
    node* target;
    node* current;

    std::deque<node*> open;   //nodes to be evaluated
    std::deque<node*> closed; //nodes that have been evaluated

public:
    void DrawOpen()
    {
        if (!open.empty())//Draw evaluted nodes
        {
            for (auto& b : open)
            {
                FillRect((b->x * BlockSize) + 1, (b->y * BlockSize) + 1, BlockSize - 1, BlockSize - 1, olc::GREEN);
            }
        }
    }

    void DrawClosed()
    {
        for (int i = 0; i < BoardHeight * BoardWidth; i++)
        {
            if (graph[i].closed == true)
            {
                FillRect((graph[i].x * BlockSize) + 1, (graph[i].y * BlockSize) + 1, BlockSize - 1, BlockSize - 1, olc::BLUE);
            }
        }
        
        
        if (!closed.empty())//Draw evaluted nodes
        {
            for (auto& b : closed)
            {
                FillRect((b->x * BlockSize) + 1, (b->y * BlockSize) + 1, BlockSize - 1, BlockSize - 1, olc::BLUE);
            }
        }
    }

    bool SetUpAstar()
    {

        open.clear();
        closed.clear();

        for (int x = 0; x < BoardWidth; x++)
        {
            for (int y = 0; y < BoardHeight; y++)
            {

                //Set all values
                int index = x + y * BoardWidth;
                graph[index].x = x;
                graph[index].y = y;
                graph[index].hcost = INFINITY;
                graph[index].gcost = INFINITY;
                graph[index].fcost = INFINITY;
                graph[index].parent = nullptr;
                graph[index].closed = false;
                if (board[index] == 2)
                    graph[index].traversable = false;
                else
                    graph[index].traversable = true;
                graph[index].vNeighbours.reserve(9);
            }
        }


        
        for (int x = 0; x < BoardWidth; x++)
        {
            for (int y = 0; y < BoardHeight; y++)
            {
                int index = x + y * BoardWidth;
                //Simple lambda for calculating offeset
                auto offset = [&](int xo, int yo)
                {
                    int in = (xo + x + (yo + y) * BoardWidth);
                    return in;
                };

                //Add Neighbours

                //Side
                if (y > 0)
                    graph[index].vNeighbours.emplace_back(&graph[offset(0, -1)]);
                if (x > 0)
                    graph[index].vNeighbours.emplace_back(&graph[offset(-1, 0)]);
                if (y < BoardHeight - 1)
                    graph[index].vNeighbours.emplace_back(&graph[offset(0, 1)]);
                if (x < BoardWidth - 1)
                    graph[index].vNeighbours.emplace_back(&graph[offset(1, 0)]);

                //Corners
                if (x > 0 && y > 0)
                    graph[index].vNeighbours.emplace_back(&graph[offset(-1, -1)]);
                if (x < BoardWidth - 1 && y > 0)
                    graph[index].vNeighbours.emplace_back(&graph[offset(1, -1)]);
                if (x < BoardWidth - 1 && y < BoardHeight - 1)
                    graph[index].vNeighbours.emplace_back(&graph[offset(1, 1)]);
                if (x > 0 && y < BoardHeight - 1)
                    graph[index].vNeighbours.emplace_back(&graph[offset(-1, 1)]);
            }
        }


        start = &graph[index_start]; //Dobrz bardzo dobrze XDD
        target = &graph[index_target];

        start->hcost = Calculate_H_Cost(start, target);
        start->gcost = 0;
        start->fcost = start->hcost + start->gcost;

        open.push_back(start);
        current = start;

        return true;

    }

    bool AstarLoop()
    {
        std::sort(open.begin(), open.end(), CompareNodes);

        if (!open.empty())
        {
            current = open.front();
            open.pop_front();
        }
        else
        {
            cout << "Error: no path" << endl;
            return false;
        }

        if (current == target)
        {
            return true;
        }

        current->closed = true;
        for (auto nodeNeighbour : current->vNeighbours)
        {
            float cost = current->gcost + Calculate_G_Cost(current, nodeNeighbour);

            if (nodeNeighbour->traversable == false || nodeNeighbour->closed == true)
                continue;

            if (cost < nodeNeighbour->gcost || !(isInSet(nodeNeighbour, open)))
            {
                nodeNeighbour->gcost = cost;
                nodeNeighbour->hcost = Calculate_H_Cost(nodeNeighbour, target);
                nodeNeighbour->fcost = nodeNeighbour->gcost + nodeNeighbour->hcost;
                nodeNeighbour->parent = current;

                if (!(isInSet(nodeNeighbour, open)))
                    open.push_back(nodeNeighbour);

            }
        }


        return true;
    }

    bool OnUserCreate() override
    {
        BoardHeight = 20;
        BoardWidth = 30;
        BlockSize = 10;
        boardChanged = false;
        runSetup = false;
        board = new int[BoardHeight * BoardWidth];
        index_start = -1;
        index_target = -1;
        start = nullptr;
        target = nullptr;
        current = nullptr;
        graph = new node[BoardWidth * BoardHeight];
        duration = 0.0f;

        memset(board, 0, sizeof(int) * BoardWidth * BoardHeight);

        return true;
    }

    bool OnUserUpdate(float fElapsedtime) override
    {


        //Controls-------------------------------------------------------------------------------------------------------------

        //Build Wall
        if (GetMouse(0).bHeld)
        {
            int index = GetMouseX() / BlockSize + GetMouseY() / BlockSize * BoardWidth;
            if (index < ScreenHeight() * ScreenWidth())
            {
                board[index] = 2;
                //boardChanged = true;
            }
        }
        //Destroy a wall
        if (GetMouse(1).bPressed)
        {
            int index = GetMouseX() / BlockSize + GetMouseY() / BlockSize * BoardWidth;
            if (index < ScreenHeight() * ScreenWidth())
            {
                board[index] = 0;
                //boardChanged = true;
            }
        }

        //Position start
        if (GetKey(olc::CTRL).bPressed)
        {
            board[index_start] = 0;
            index_start = GetMouseX() / BlockSize + GetMouseY() / BlockSize * BoardWidth;
            board[index_start] = 3;
        }
        //Position end
        if (GetKey(olc::SHIFT).bPressed)
        {
            board[index_target] = 0;
            index_target = GetMouseX() / BlockSize + GetMouseY() / BlockSize * BoardWidth;
            board[index_target] = 4;
        }

        //Reset the board
        if (GetKey(olc::R).bPressed)
        {
            memset(board, 0, BoardHeight * BoardWidth * sizeof(int));
        }

        ////Set G and H costs
        //if (GetKey(olc::Q).bPressed)
        //    Gcost_m += 0.1f;
        //if (GetKey(olc::A).bPressed)
        //    Gcost_m -= 0.1f;
        //if (GetKey(olc::W).bPressed)
        //    Hcost_m += 0.1f;
        //if (GetKey(olc::S).bPressed)
        //    Hcost_m -= 0.1f;


        //Run Astar
        if (GetKey(olc::SPACE).bPressed)
        {
            boardChanged = true;
            SetUpAstar();
        }

        if (boardChanged)
        {
            //Do the path fiding
            if (index_start != -1 && index_target != -1)
            {
                if (current != target)
                {
                    
                    AstarLoop();                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(7));
                }
                else
                {
                    boardChanged = false;
                    std::cout << "Pathfinding time: " << duration << " ms" << std::endl;
                    duration = 0.0f;
                }
                    
            }
        }


        Clear(olc::BLACK);
        //Draw the board
        for (int x = 0; x < BoardWidth; x++)
        {
            for (int y = 0; y < BoardHeight; y++)
            {
                if (board[x + y * BoardWidth] == 0)
                    FillRect((x * BlockSize) + 1, (y * BlockSize) + 1, BlockSize - 1, BlockSize - 1, olc::WHITE);
            }
        }


        DrawClosed();
        DrawOpen();

        for (int x = 0; x < BoardWidth; x++)//Draw walls
        {
            for (int y = 0; y < BoardHeight; y++)
            {
                if (board[x + y * BoardWidth] == 2)
                    FillRect((x * BlockSize) + 1, (y * BlockSize) + 1, BlockSize - 1, BlockSize - 1, olc::DARK_GREY);
            }
        }


        if (target != nullptr)//Draw the path
        {
            node* p = target;
            while (p->parent != nullptr)
            {
                FillRect((p->x * BlockSize) + 1, (p->y * BlockSize) + 1, BlockSize - 1, BlockSize - 1, olc::YELLOW);
                p = p->parent;
            }
        }


        int x_s = index_start % BoardWidth;
        int y_s = index_start / BoardWidth;
        int x_t = index_target % BoardWidth;
        int y_t = index_target / BoardWidth;
        FillRect((x_s * BlockSize) + 1, (y_s * BlockSize) + 1, BlockSize - 1, BlockSize - 1, olc::GREEN);
        FillRect((x_t * BlockSize) + 1, (y_t * BlockSize) + 1, BlockSize - 1, BlockSize - 1, olc::RED);



        return true;
    }
};

int main()
{
    engine demo;
    if (demo.Construct(300, 200, 5, 5))
        demo.Start();
}
