import QtQuick
import QtGraphs
import "./lib.js" as JSL

GraphsView {
    id: root
    required property var dataProvider
    anchors.fill: parent

    Connections{
        target: dataProvider
        function onUsageChanged(v){
            // dbg("onUsageChanged data=" + v[v.length-1] + " RAM=" + ( dataProvider.totalRAM / 1024))
            memUsage.clear()
            memUsage.append(v)
        }
    }
    function dbg(str, code ="") {
        console.log( String("%1:[MemView.qml] %2").arg(code).arg(str));
    }
    // theme: GraphsTheme {
        // baseGradients: [ Gradient {
        //     GradientStop { position: 1.0; color: "#DBEB00" }
        //     GradientStop { position: 0.0; color: "#373F26" }
        // } ]
        // seriesColors: ["salmon"]
        // colorStyle: GraphsTheme.ColorStyle.Uniform
        // colorStyle: GraphsTheme.ColorStyle.ObjectGradient
        // colorStyle: GraphsTheme.ColorStyle.RangeGradient
        // gridVisible: false
        // grid.mainColor: "red"
        // grid.subColor: "blue"
    // }
    axisX: ValueAxis {
        min:-60
        max: 0
        gridVisible: false
        titleText: qsTr("RAM usage") + "(of " + JSL.humanMem(dataProvider.totalRAM) + ")"
    }
    axisY: ValueAxis {
        visible: false
        max: dataProvider.totalRAM / 1024
        labelDecimals :0
        tickInterval: max/4
        gridVisible: false
        lineVisible: false
    }

    LineSeries {
        id: memUsage
        // color: "tomato"
        color: "salmon"
    }

    // AreaSeries{
    //     opacity: 0.2
    //     upperSeries: LineSeries {
    //         id: memUsage
    //     }
    // }
}
