#ifndef GAMEFRAME_H
#define GAMEFRAME_H

#include <QFrame>
#include <world/World.h>
#include "world/Point.h"
#include "imagerepository.h"
namespace Ui {
class GameFrame;
}

class GameWindow;

class GameFrame : public QFrame
{
    Q_OBJECT

public:
    explicit GameFrame(QWidget *parent = nullptr);
    ~GameFrame();


    void paintEvent(QPaintEvent* event) override;

    void setWindow(GameWindow* win);
    friend class GameWindow;
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void wheelEvent(QWheelEvent* event) override;
private:
    z2::ImageRepository& repo = z2::ImageRepository::instance();

    Ui::GameFrame *ui;
    GameWindow* win;
    z2::Point selPos;
    QTransform trans;

    QPoint clickedPos;
    int dragState = 0;
    QPoint gameCordToViewCord(const z2::Point& p);

    z2::Point viewCordToGameCord(const QPoint& p);


    void paintTile(QPainter &painter, QBrush &brush, QRect &rect, z2::Tile &t, z2::Player& p);

    void paintTiles(QPainter &painter, const shared_ptr<z2::World> &world);

    void paintHighlightedTile(QPainter& painter, const shared_ptr<z2::World> &world );

    void rightClickedOn(z2::Point& p);

    void zoom(bool in);
    void paintTerrainTextureLost(QPainter &painter, QBrush &brush, QRect &rect, z2::Tile &t);
    void paintTerrain(QPainter &painter, QBrush &brush, QRect &rect, z2::Tile &t);

    void paintResource(QPainter &painter, QBrush &brush, QRect &rect, z2::Tile &t);

    void paintEntity(QPainter &painter, QBrush &brush, QRect &rect, z2::Tile &t, bool onlyConstruction);
};

#endif // GAMEFRAME_H
