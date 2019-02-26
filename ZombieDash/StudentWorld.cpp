#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include "Level.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;


GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    citizen_count=0;
    zombie_count=0;
}

StudentWorld::~StudentWorld()
{
    cleanUp();
    
}

bool determineNewPosition(Direction dir, double x, double y, double distance)
{
    
    switch(dir)
    {
        case Actor::left:
            x-=distance;
            break;
        case Actor::right:
            x+=distance;
            break;
        case Actor::up:
            y+=distance;
            break;
        case Actor::down:
            y-=distance;
            break;
    }
    bool X_OutOfBound = x<0 || x >= VIEW_WIDTH;
    bool Y_OutOfBound = y<0 || y >= VIEW_HEIGHT;
    
    return X_OutOfBound || Y_OutOfBound;
    
}

//Check whether the new location is still in StudentWorld
//Returns false if x y coordinates are out of bound, with x,y remaining unchanged.
//Returns true otherwise, and set x,y to new values
bool StudentWorld::getNewPositionWithDir(Direction dir, double& x, double& y)
{
    if( dir==Actor::left || dir == Actor::right )
        return determineNewPosition(dir, x, y, SPRITE_WIDTH);
    else return determineNewPosition(dir, x, y, SPRITE_HEIGHT);
}


int StudentWorld::init()
{
    Level lev(assetPath());
    //Create string containing name of file
    ostringstream oss;
    oss.setf(ios::fixed);
    oss.precision(2);
    oss<<"level";
    oss.fill('0');
    oss<<setw(2)<<getLevel();
   
    oss<<".txt";
    string levelFile=oss.str();
    
    Level::LoadResult result=lev.loadLevel(levelFile);
    if(result == Level::load_fail_file_not_found)
        cerr<<"Cannot find level01.txt data file"<<endl;
    else if(result == Level::load_fail_bad_format)
        cerr<<"Your level was improperly formatted"<<endl;
    else if (result == Level::load_success)
    {
        for(int y=0;y<LEVEL_HEIGHT;y++)
        {
            for(int x=0;x<LEVEL_WIDTH;x++)
            {
                Level::MazeEntry ge = lev.getContentsOf(x, y);
                switch(ge){
                    case Level::wall:{
                        Actor *actor = new Wall(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                        m_actors.push_back(actor);
                    }break;
                        
                    case Level::player:
                    {
                        pene = new Penelope(this, x*SPRITE_WIDTH, y*SPRITE_HEIGHT);
                    }break;
                    
                    case Level::citizen:{
                        Actor* actor = new Citizen(this,x*SPRITE_WIDTH,y*SPRITE_HEIGHT);
                        m_actors.push_back(actor);
                        
                    }break;
                    
                    case Level::pit:{
                        Actor* actor = new Pit(this,x*SPRITE_WIDTH,y*SPRITE_HEIGHT);
                        m_actors.push_back(actor);
                        
                    }break;
                        
                    case Level::vaccine_goodie:{
                        Actor* actor = new VaccineGoodie(this,x*SPRITE_WIDTH,y*SPRITE_HEIGHT);
                        m_actors.push_back(actor);
                        
                    }break;
                        
                    case Level::gas_can_goodie:{
                        Actor* actor = new GasCanGoodie(this,x*SPRITE_WIDTH,y*SPRITE_HEIGHT);
                        m_actors.push_back(actor);
                        
                    }break;
                        
                    case Level::landmine_goodie:{
                        Actor* actor = new LandmineGoodie(this,x*SPRITE_WIDTH,y*SPRITE_HEIGHT);
                        m_actors.push_back(actor);
                        
                    }break;
                        
                    case Level::exit:{
                        Actor* actor = new Exit(this,x*SPRITE_WIDTH,y*SPRITE_HEIGHT);
                        m_actors.push_back(actor);
                        
                    }break;
                        
                    case Level::dumb_zombie:{
                        Actor* actor = new DumbZombie(this,x*SPRITE_WIDTH,y*SPRITE_HEIGHT);
                        m_actors.push_back(actor);
                        
                    }break;
                        
                    case Level::smart_zombie:{
                        Actor* actor = new SmartZombie(this,x*SPRITE_WIDTH,y*SPRITE_HEIGHT);
                        m_actors.push_back(actor);
                        
                    }break;
                        
                    
                    
                }
                
                
            }
            
        }
        
        
        
        
    }
    return GWSTATUS_CONTINUE_GAME;
}

//##########################
//MARK: - Move
//##########################

int StudentWorld::move()
{
    updateTick();
    
    //move all Actors in a for-loop
    if(getLives()>0)
        pene->doSomething();
    for(list<Actor*>::iterator it=m_actors.begin(); it!=m_actors.end(); it++)
    {
        if((*it)->isAlive())
            (*it)->doSomething();
        
        //Penelope dies
        if(getLives()==0)
        {
            playSound(SOUND_PLAYER_DIE);
            return GWSTATUS_PLAYER_DIED;
        }
        
        
         //if citizen Penelope has escaped
         if(gameFinished)
         {
             playSound(SOUND_LEVEL_FINISHED);
             return GWSTATUS_FINISHED_LEVEL;
         }
        
    }
    for(list<Actor*>::iterator it=m_actors.begin(); it!=m_actors.end(); it++)
    {
        //Actor dies
        if( ! (*it)->isAlive())
        {
            delete *it;
            *it=nullptr;
            m_actors.erase(it);
        }
    }
    
    
    
   
    
    return GWSTATUS_CONTINUE_GAME;
    
}

//Check whether the given position is in StudentWorld
//AND NOT being blocked by an exit or a Wall
bool StudentWorld::isFlameBlockedAt(double x, double y) const
{
    bool flag=false;
    for(list<Actor*>::const_iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        if((*it)->blocksFlame() && checkOverlapByOneObject(x, y, (*it)))
            flag=true;
    }
    return flag;
}

void StudentWorld::killByFlameIfAppropriate(Flame* flame)
{
    for(list<Actor*>::iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        if((*it)->canKillByFlame() && checkOverlapByTwoObjects(flame, (*it)))
        {
            (*it)->setDead();
            
        }
        
    }
    
}

void StudentWorld::infectByVomitIfAppropriate(Vomit* vomit)
{
    for(list<Actor*>::iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        vomit->infectByVomitIfAppropriate(*it);
    }
}

void StudentWorld::introduceFlameIfAppropriate(Landmine* landmine, double x, double y, Direction dir)
{
    if(!isFlameBlockedAt(x, y))
    {
        addActor(new Flame(this, x,y, Actor::up));
    }
}

void StudentWorld::introduceFlameIfAppropriate(Landmine* landmine, double x, double y)
{
    if(getNewPositionWithDir(up, <#double &x#>, <#double &y#>))
}

bool StudentWorld::locateNearestVomitTrigger(double x, double y, Actor* &target, double& distance)
{
    double min_dist;
    double target_x,target_y;
    for(list<Actor*>::const_iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        if((*it)->canInfectByVomit())
        {
            target_x=(*it)->getX();
            target_y=(*it)->getY();
            distance=(x-target_x)*(x-target_x)+(y-target_y)*(y-target_y);
            if(distance<min_dist)
            {
                min_dist=distance;
                target=(*it);
            }
        }
    }
    return min_dist<=100;
    
}

bool StudentWorld::isAgentMovementBlockedAt(Agent* ag, double x, double y) const
{
    for(list<Actor*>::const_iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        if((*it)->blocksAgent() && checkOverlapByOneObject((*it), x, y));
        {
            return true;
        }
    }
    return false;
    
}


////Check whether a - the Actor passed in - overlap with a specific position
//bool StudentWorld::checkAllOverlap(Actor* a, int X, int Y)
//{
//    for(list<Actor*>::iterator it=m_actors.begin();it!=m_actors.end();it++)
//    {
//        if((*it) == a) continue;
//        if(a->checkOverlap(*it, X, Y))
//            return true;
//    }
//    return false;
//    
//    
//}

//##########################
//MARK: - cleanUp
//##########################

void StudentWorld::cleanUp()
{
        for(std::list<Actor*>::iterator it=m_actors.begin();it != m_actors.end();it++)
        {
            delete *it;
            (*it) = nullptr;
        }
        m_actors.erase(m_actors.begin(),m_actors.end());
        delete pene;
        pene=nullptr;
}
