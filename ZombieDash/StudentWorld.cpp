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
: GameWorld(assetPath),gameFinished(false)
{
    citizen_count=0;
    zombie_count=0;
    infect_count=0;
}

StudentWorld::~StudentWorld()
{
    cleanUp();
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
                        zombie_count++;
                        
                    }break;
                        
                    case Level::smart_zombie:{
                        Actor* actor = new SmartZombie(this,x*SPRITE_WIDTH,y*SPRITE_HEIGHT);
                        m_actors.push_back(actor);
                        zombie_count++;
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
    writeStatus();
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
            return GWSTATUS_PLAYER_DIED;
        }
    }
    
    //if citizen Penelope has escaped
    if(citizen_count==0 && gameFinished)
    {
        playSound(SOUND_LEVEL_FINISHED);
        return GWSTATUS_FINISHED_LEVEL;
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

//
void StudentWorld::activateOnAppropriateActors(Actor* a)
{
    for(list<Actor*>::iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        a->activateIfAppropriate(*it);
    }
}

void StudentWorld::introduceFlameIfAppropriate(double x, double y)
{
    if(!isFlameBlockedAt(x, y))
    {
        addActor(new Flame(this, x,y, Actor::up));
    }
}

void StudentWorld::useExit(Exit* exit)
{
    pene->useExitIfAppropriate(exit);
    for(list<Actor*>::iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        (*it)->useExitIfAppropriate(exit);
    }
}

bool StudentWorld::locateNearestVomitTrigger(double x, double y, Actor* &target, double& distance)
{
    double dist;
    distance=INT_MAX;
    double target_x=INT_MAX,target_y=INT_MAX;
    for(list<Actor*>::const_iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        if((*it)->canInfectByVomit())
        {
            target_x=(*it)->getX();
            target_y=(*it)->getY();
            dist=(x-target_x)*(x-target_x)+(y-target_y)*(y-target_y);
            if(distance>dist)
            {
                distance=dist;
                target=(*it);
            }
        }
    }
    return distance<=100;
    
}

bool StudentWorld::locateNearestCitizenTrigger(double zombie_x, double zombie_y, double& target_x, double& target_y, Actor* target, double& distance, bool& isThreat) const
{
    double dist;
    distance=INT_MAX;
    
    for(list<Actor*>::const_iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        if((*it)->canInfectByVomit())
        {
            double x=(*it)->getX();
            double y=(*it)->getY();
            dist=(zombie_x-x)*(zombie_x-x)+(zombie_y-y)*(zombie_y-y);
            if(distance>dist)
            {
                distance=dist;
                target=(*it);
                target_x=x;
                target_y=y;
            }
        }
    }
    return distance<=6400;
}

//Locate nearest Zombie to the Citizen
//Called by Citizen
//Returns true if there is zombie closer than 80 pixels
bool StudentWorld::locateNearestCitizenThreat(double citizen_x, double citizen_y, double& otherX, double& otherY, double& dist_z)
{
    double dist;
    dist_z=INT_MAX;
    double x=INT_MAX, y=INT_MAX;
    for(list<Actor*>::iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        if((*it)->isZombie())
        {
            
            x=(*it)->getX();
            y=(*it)->getY();
            
            double distance=(citizen_x-x)*(citizen_x-x)+(citizen_y-y)*(citizen_y-y);
            if(distance<dist)
            {
                dist_z=distance;
                otherX=x;
                otherY=y;
            }
        }
    }
    return dist_z<=6400;
    
}

//Determine whether attempted move is blocked
bool StudentWorld::isBlocked(double a_x, double a_y, double c_x, double c_y) const
{
    double b_x=a_x+SPRITE_WIDTH-1, b_y=a_y+SPRITE_HEIGHT-1;
    double d_x=c_x+SPRITE_WIDTH-1, d_y=c_y+SPRITE_HEIGHT-1;
    return !(a_x>d_x || b_x<c_x || a_y>d_y || b_y<c_y);
    
}

bool StudentWorld::isAgentMovementBlockedAt(Agent* ag, double x, double y) const
{
    for(list<Actor*>::const_iterator it=m_actors.begin();it!=m_actors.end();it++)
    {
        //TODO:
        if((*it)->blocksAgent() && isBlocked(x,y,(*it)->getX(), (*it)->getY()) && (*it)!=ag)
        {
            return true;
        }
    }
    return false;
    
}

void StudentWorld::writeStatus()
{
    //eg: Score: 004500 Level: 27 Lives: 3 Vaccines: 2 Flames: 16 Mines: 1 Infected: 0
    ostringstream oss;
    
    //Display Score
    oss<<"Score: ";
    oss.fill('0');
    oss<<setw(6)<<getScore()<<"  ";
    
    //Display Level
    oss<<"Level:  "<<getLevel()<<"  ";
    
    //Display Lives
    oss<<"Lives:  "<<getLives()<<"  ";
    
    //Display Vaccines
    oss<<"Vaccines:  "<<pene->getNumVaccines()<<"  ";
    
    //Display Flames
    oss<<"Flames:  "<<pene->getNumFlameCharges()<<"  ";
    
    //Display Mines
    oss<<"Mines:  "<<pene->getNumLandmines()<<"  ";
    
    //Display Number of infected citizens
    oss<<"Infected: "<<infect_count;
    
    setGameStatText(oss.str());
    
}

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
