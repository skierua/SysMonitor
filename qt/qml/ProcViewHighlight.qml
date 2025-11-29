import QtQuick

Rectangle {
    id: root
    width: root.ListView.view.width;
    height: 27
    color: "lightsteelblue";
    radius: 5
    y: root.ListView.view.currentItem === null ? null : root.ListView.view.currentItem.y
    Behavior on y {
        SpringAnimation {
            spring: 3
            damping: 0.2
        }
    }
}
