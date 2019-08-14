#include "gameframe.h"
#include "ui_gameframe.h"
#include "gamewindow.h"
#include "core/messages/UnitAttack.h"
#include "core/messages/UnitMove.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>

#include <entity/Building.h>
#include <config/EntityRepository.h>

#include "playercolor.h"

using namespace z2;

const static QSize qTileSize(TILE_SIZE,TILE_SIZE);
const static QSize qPlayerColorSize(PLAYER_COLOR_BLOCK_SIZE,PLAYER_COLOR_BLOCK_SIZE);
const static bool GF_DEBUG = false;

GameFrame::GameFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::GameFrame)
{
    ui->setupUi(this);
    trans.scale(1.5,1.5);
}

GameFrame::~GameFrame()
{
    delete ui;
}

void paintLines(QPainter& painter, const shared_ptr<World>& world){
    int wH = world->getHeight();
    int wW = world->getWidth();
    int xEnd = wW * TILE_SIZE;
    int yEnd = wH * TILE_SIZE;
    QPen pen;
    pen.setWidth(2);
    painter.setPen(pen);
    for(int i=0;i<=wH;i++){
        int y = i*TILE_SIZE;
        painter.drawLine(0,y,xEnd,y);
    }
    for(int i=0;i<=wW;i++){
        int x = i*TILE_SIZE;
        painter.drawLine(x,0,x,yEnd);
    }
}

QPoint GameFrame::gameCordToViewCord(const Point &p)
{
    auto w = win->getWorld();
    int y = w->getHeight() - p.y - 1;
    int x = p.x;
    return QPoint(x * TILE_SIZE, y * TILE_SIZE);
}

Point GameFrame::viewCordToGameCord(const QPoint &p)
{
    auto w = win->getWorld();
    auto invt = trans.inverted();
    auto pReal = invt.map(p);
    int x = pReal.x() / TILE_SIZE;
    int y = w->getHeight() -  (pReal.y() / TILE_SIZE) - 1;
    return Point(x,y);
}

void GameFrame::paintResource(QPainter &painter, QBrush &brush, QRect &rect, Tile &t)
{
    string name;
    switch(t.getResource()){
    case Resource::GEM:{
        name = "Resource_Gem.png";
        break;
    }
    case Resource::MINE:{
        name = "Resource_Mine.png";
        break;
    }
    default:{
        name = "None";
        break;
    }
    }
    if(repo.containsImage(name)){
        const ImagePtr& image = repo.getImage(name);
        painter.drawImage(rect,*image);
    }
}


void GameFrame::paintTerrain(QPainter& painter,QBrush& brush, QRect& rect, Tile& t){
    string name;
    switch (t.getTerrain()) {
    case Terrain::PLAIN:{
        name = "Terrain_Plain.png";
        break;
    }
    case Terrain::MOUNTAIN:{
        name = "Terrain_Mountain.png";
        break;
    }
    case Terrain::HILL:{
        name ="Terrain_Hill.png";
        break;
    }
    default:{
        name = "Null";
        break;
    }
    }
    if(!repo.containsImage(name)){
        paintTerrainTextureLost(painter,brush,rect,t);
    }else{
        const ImagePtr& image = repo.getImage(name);
        painter.drawImage(rect,*image);
    }
}


QColor getColor(int playerId, World& w){
    if(playerId == Player::NO_PLAYER){
        return QColor(Qt::white);
    }
    int cc = w.getPlayer(playerId).getColorCode();

    return PlayerColor::getColor(cc);
}

void GameFrame::paintEntity(QPainter &painter, QBrush &brush, QRect &rect,World& world, Tile &t, bool grey)
{
    auto en = t.getEntity();
    if(!en){
        return;
    }

    shared_ptr<Building> building = dynamic_pointer_cast<Building>(en);
    if(grey && (!building)){
        return;
    }
    auto& info = en->getEntityInfo();
    auto& imageName = info.getImageName();
    if(repo.containsImage(imageName)){
        const ImagePtr& image = repo.getImage(imageName);
        painter.drawImage(rect,*image);
    }else{
        painter.drawText(rect,QString::fromStdString(info.getDisplayName()));
    }
    // draw player color
    if(!grey){
        int owner = en->getOwnerId();
        QColor c = getColor(owner,world);
        QRect sr(rect.topLeft(),qPlayerColorSize);
        brush.setColor(c);
        painter.fillRect(sr,brush);
    }
}



void GameFrame::paintTerrainTextureLost(QPainter& painter,QBrush& brush,QRect& rect, Tile& t){
    switch (t.getTerrain()) {
    case Terrain::PLAIN:{
        brush.setColor(Qt::darkGreen);
        break;
    }
    case Terrain::MOUNTAIN:{
        brush.setColor(Qt::gray);
        break;
    }
    default:{
        brush.setColor(Qt::black);
        break;
    }
    }
    painter.fillRect(rect,brush);
}



void GameFrame::paintTile(QPainter &painter, QBrush &brush, QRect &rect,World& world, Tile &t, Player &p)
{
    auto v = t.getVisibility(p.getPlayerId());
    bool grey = false;
    if(!GF_DEBUG){
        switch(v){
        case Visibility::DARK:{
            brush.setColor(Qt::darkGray);
            painter.fillRect(rect,brush);
            return;
        }
        case Visibility::GREY:{
            grey = true;
            break;
        }
        case Visibility::CLEAR:{
            break;
        }
        }
    }

    paintTerrain(painter,brush,rect,t);
    paintResource(painter,brush,rect,t);
    paintEntity(painter,brush,rect,world,t,grey);
    if(grey){
        brush.setColor(QColor(100,100,100,128));
        painter.fillRect(rect,brush);
    }
}


void GameFrame::paintTiles(QPainter& painter, const shared_ptr<World>& world){
    int wH = world->getHeight();
    int wW = world->getWidth();
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    Player& p =world->getPlayer(win->getClient()->getPlayerId());
    for(int x=0;x<wW;x++){
        for(int y=0;y<wH;y++){
            Tile& t = world->getTile(x,y);
            QPoint ul = gameCordToViewCord(Point(x,y));
            auto rect = QRect(ul,qTileSize);
            paintTile(painter,brush,rect,*world,t,p);
        }
    }
}

void GameFrame::paintHighlightedTile(QPainter &painter, const shared_ptr<World> &world)
{
    if(!world->isInside(selPos)){
        return;
    }
    QPoint ul = gameCordToViewCord(selPos);
    auto rect = QRect(ul,qTileSize);
    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::red);
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    painter.drawRect(rect);
}




void GameFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setTransform(trans);
//    painter.setWindow(QRect(20,20,int(percent * 631),int(percent *551)));
    auto world = win->getWorld();
    if(!world){
        return;
    }
    paintTiles(painter,world);
    paintLines(painter,world);
    paintHighlightedTile(painter,world);
}

void GameFrame::setWindow(GameWindow *w)
{
    win = w;
}



void GameFrame::mousePressEvent(QMouseEvent *event)
{
//    cout << "Event" << endl;
    switch(event->button()){
    case Qt::MouseButton::LeftButton:{
        auto pos = event->pos();
        clickedPos = pos;
        clickedTime = clock();
        dragState = 1;
        break;
    }
    case Qt::MouseButton::RightButton:{
        auto pos = event->pos();
        auto pGame = viewCordToGameCord(pos);
        rightClickedOn(pGame);
//        win->update();
        break;
    }

    }
}
const int MinTimeGapMilliseconds = 100;
void GameFrame::mouseMoveEvent(QMouseEvent *event)
{
    if(dragState == 0){
        return;
    }
    clock_t cur = clock();
    if(cur - clickedTime <= MinTimeGapMilliseconds * CLOCKS_PER_SEC / 1000){
        return;
    }
    dragState = 2;
    auto pos = event->pos();
    int dx = pos.x() - clickedPos.x();
    int dy = pos.y() - clickedPos.y();
    clickedPos = pos;
    trans.translate(dx,dy);
    update();
}

void GameFrame::mouseReleaseEvent(QMouseEvent *event)
{
    switch(event->button()){
    case Qt::MouseButton::LeftButton:{
        if(dragState != 2){
            auto pos = event->pos();
            selPos = viewCordToGameCord(pos);
//            cout << "Selected pos: " << selPos.x << ' ' << selPos.y << endl;
            win->refreshSelection(true);
            update();
        }
        break;
    }
    }
    dragState = 0;
}

void GameFrame::rightClickedOn(Point &p)
{
    auto world = win->getWorld();
    auto client = win->getClient();
    auto en = world->getEntity(selPos);
    bool operated = false;
    if(world->canAttack(selPos,p,client->getPlayerId())){
        shared_ptr<UnitAttack> msg(new UnitAttack(selPos,p));
        client->sendMessageToServer(msg);
        operated = true;
    }else if (world->canMove(selPos,p,client->getPlayerId())) {
        shared_ptr<UnitMove> msg(new UnitMove(selPos,p));
        client->sendMessageToServer(msg);
        operated = true;
    }
    if(operated){
        selPos = Point(-1,-1);
        win->refreshSelection();
//        win->refreshAll();
    }

}

void GameFrame::zoom(bool in)
{
    double mul;
    if(in){
        mul = 1.25;

    }else{
        mul = 0.8;
    }
    trans.scale(mul,mul);
    update();
}



void GameFrame::wheelEvent(QWheelEvent *event)
{
    this->zoom(event->delta() > 0);
}


