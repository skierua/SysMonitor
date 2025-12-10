import QtQuick
import QtQuick.Templates as T

T.ItemDelegate {
    id: root
    width: root.ListView.view.headerItem.width
    height: contentItem.height;
    // implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
    //                         implicitContentWidth + leftPadding + rightPadding)
    // implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
    //                          implicitContentHeight + topPadding + bottomPadding)

    highlighted: ListView.isCurrentItem

    required property int row
    required property var model

    signal openContextMenu()
    // signal openContextMenu(real vx, real vy)
    signal crntPIDChanged()

    background: Rectangle {
        // border.width: control.current ? 2 : Qt.styleHints.accessibility.contrastPreference === Qt.HighContrast ? 1 : 0
        // border.color: control.current ? control.palette.highlight : control.palette.windowText
        color: root.highlighted
               ? "LightSteelBlue"
               : (root.row % 2 !== 0
               ? root.palette.alternateBase : root.palette.base)
    }

    contentItem: Item{
        width: root.width
        height: 25
        // Row {
            Row{
                anchors{fill: parent; margins: 2}
                // width: parent.width - parent.children[1].width
                spacing: 2
                Text{
                    width: parent.width
                           * root.ListView.view.headerItem.children[0].children[0].width
                           / root.ListView.view.headerItem.children[0].width
                           - 2      //parent.anchors.leftMargin
                    text: root.model.pid
                }
                Text{
                    width: parent.width
                           * root.ListView.view.headerItem.children[0].children[1].width
                           / root.ListView.view.headerItem.children[0].width
                    // required property string comm
                    text: root.model.comm
                }
                Text{
                    width: parent.width
                           * root.ListView.view.headerItem.children[0].children[2].width
                           / root.ListView.view.headerItem.children[0].width
                    text: root.model.mem
                    horizontalAlignment: Text.AlignHCenter
                }
                Text{
                    width: parent.width
                           * root.ListView.view.headerItem.children[0].children[3].width
                           / root.ListView.view.headerItem.children[0].width
                    text: root.model.tm
                }
                Text{
                    width: parent.width
                           * root.ListView.view.headerItem.children[0].children[4].width
                           / root.ListView.view.headerItem.children[0].width
                           - 2      //parent.anchors.rightMargin
                    text: root.model.th_str
                    // text: root.model.th_all + "/" + root.model.th_active
                    horizontalAlignment: Text.AlignHCenter
                }

            }
            // Rectangle{
            //     id: menuItem
            //     // visible: false
            //     width: visible ? 50 : 0
            //     height: parent.height
            //     color: "salmon"
            //     Text{
            //         text: "KILL"
            //     }
            // }
        // }

        MouseArea{
            anchors.fill: parent
            onClicked: (mouse) => {
                // console.log("T.ItemDelegate clicked pid =" +root.model.pid + " index="+ root.row )
                if (root.ListView.view.model.crntPID !== root.model.pid) {
                   root.ListView.view.model.crntPID = root.model.pid
                   // should set the currentIndex becouse Providet set it in a second
                   root.ListView.view.currentIndex = root.row
                   crntPIDChanged() // emit signal
                   if (mouse.modifiers & Qt.ShiftModifier) openContextMenu()
                } else {
                    root.ListView.view.currentIndex = -1
                   root.ListView.view.model.crntPID = 0
                }
            }
            // onDoubleClicked: {
            //     // console.log("T.ItemDelegate clicked pid =" +root.model.pid + " index="+ root.row )
            //     root.ListView.view.model.crntPID = root.model.pid
            //     root.ListView.view.currentIndex = root.row
            //         // openContextMenu()
            //     // openContextMenu(mouse.x, mouse.y)
            // }
        }
    }
    // Component.onCompleted:{
    //     if (pid === root.ListView.view.model.crntPID) {
    //         root.ListView.view.setCrntIndex(index)
    //                             // root.ListView.view.currentIndex = index;
    //         dbg(String("delegate Component.onCompleted index=%1 pid=%2 crntPID=%3 currentIndex=%4")
    //             .arg(index).arg(pid).arg(root.ListView.view.model.crntPID).arg(root.ListView.view.currentIndex) )
    //     }
    // }
}
