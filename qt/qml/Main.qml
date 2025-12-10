import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


import SysMonitor 1.0

Window {
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("SystemMonitor")
    // required property var procData
    // onProcDataChanged: dbg("length="+ procData.length)
    // required property AbstractItemModel procModel
    // onProcModelChanged: dbg("size="+ procModel.count)
    // onProcModelChanged: dbg("size="+ procModel.count)
    required property AbstractItemModel procProvider
    required property var memProvider

    Connections{
        target: procProvider
        // function onModelReset() {
        //     // vw.restoreIndex()
        // }
        function onCrntPIDIndexChanged(index){
            // dbg("onIndexForCrntPIDChanged index=" + index)
            vw.currentIndex = index
        }

        // function onEmitTest(data) {dbg("test testProcData="+ procProvider.testProcData()[0].pid);}
    }

    function dbg(str, code ="") {
        console.log( String("%1:[Main.qml] %2").arg(code).arg(str));
    }
    Action{
        id: terminateAction
        text: qsTr("Terminate")
        onTriggered: procProvider.terminate()
            // dbg("terminateAction onTriggered PID=" + procProvider.crntPID)
    }

    Page{
        anchors.fill: parent
        // header: ToolBar{
        //     anchors.margins: 5
        //     RowLayout {

        //     }
        // }
        Pane{
            anchors.fill: parent
            ColumnLayout{
                anchors.fill: parent
                Item{
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ListView{
                        id: vw
                        anchors.fill: parent
                        currentIndex: -1
                        model: root.procProvider
                        spacing: 2
                        // cacheBuffer: 200
                        header: Rectangle{
                            width: vw.width
                            height: 30
                            // color: "lightgrey"
                            Row{
                                width: parent.width
                                anchors{ verticalCenter: parent.verticalCenter }
                                HeaderElem{
                                    width: parent.width * 0.1 - parent.spacing
                                    htext: qsTr("PID")
                                }
                                HeaderElem{
                                    width: parent.width * 0.4 - parent.spacing
                                    htext: qsTr("Command")
                                }
                                HeaderElem{
                                    width: parent.width * 0.15 - parent.spacing
                                    htext: qsTr("Memory")
                                }
                                HeaderElem{
                                    width: parent.width * 0.2 - parent.spacing
                                    htext: qsTr("Time")
                                }
                                HeaderElem{
                                    width: parent.width * 0.15
                                    htext: qsTr("Th/A")
                                }
                            }
                        }
                        ScrollBar.vertical: ScrollBar{
                            parent: vw.parent
                            anchors.top: vw.top
                            anchors.left: vw.right
                            anchors.bottom: vw.bottom
                        }
                        delegate: ProcViewDelegate{
                            onOpenContextMenu: { procContextMenu.popup(); }
                            onCrntPIDChanged: { footerCrntPath.text = procProvider.procPath(); }
                        }
                        // highlight: ProcViewHighlight{}
                        // highlightFollowsCurrentItem: false
                        // focus: true

                        /*
                        //too slow, moved to c++
                        function restoreIndex(){
                            let crnt = -1
                            const pid = model.crntPID
                            if (pid === 0) {
                                currentIndex = crnt
                                return
                            }
                            for (let i =0; i < model.rowCount(); ++i) {
                                if (model.getPID(i) === pid) {
                                    crnt = i
                                    break;
                                }
                            }
                            // dbg(String("pid=%1 i=%2 model.pid=%3 count=%4")
                            //     .arg(pid).arg(i).arg(model.getPID(i)).arg(model.rowCount()))
                            currentIndex = crnt;
                        }
                        */

                        onCurrentIndexChanged: {
                            terminateAction.enabled = model.canTerminate()
                        }
                        onCountChanged: footerTotal.text = String("%1 proc").arg(count)

                    }
                    Menu{
                        id: procContextMenu
                        // MenuItem { action: clearFilterAction; }
                        MenuItem { action: terminateAction; }
                        // MenuSeparator { padding: 5; }
                        // MenuItem { action: bindModeAction; }
                    }
                    // MouseArea{
                    //     anchors.fill: parent
                    //     onClicked: {procContextMenu.open()}
                    // }

                }
                Item{
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height / 3
                    Layout.maximumHeight: 200
                    clip: true
                    MemView{
                        dataProvider: root.memProvider
                    }
                }

            }
        }
        footer:  Rectangle{
            width: parent.width
            height: 30      //childrenRect.height
            color: 'whitesmoke'
            RowLayout{
                anchors{fill: parent; margins: 5}
                spacing: 5
                Text {
                    id: footerCrntPath
                    Layout.fillWidth: true
                    elide: Text.ElideLeft
                    MouseArea{
                        anchors.fill: parent
                        hoverEnabled: true
                        ToolTip.delay: 1000
                        ToolTip.timeout: 5000
                        ToolTip.visible: containsMouse
                        ToolTip.text: parent.text
                    }
                }
                Text {
                    Layout.minimumWidth: 0.1 * parent.width
                    id: footerTotal
                }

            }

        }

    }

}
