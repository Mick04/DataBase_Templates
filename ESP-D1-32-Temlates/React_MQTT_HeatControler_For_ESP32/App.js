// In App.js in a new project
import * as React from "react";
import { NavigationContainer } from "@react-navigation/native";
import { SafeAreaView, StyleSheet, Platform, StatusBar } from "react-native";
import { createMaterialTopTabNavigator } from "@react-navigation/material-top-tabs";

import SettingsScreen from "./components/Settings.js";
import GaugeScreen from "./components/Gauges.js";
import HomeScreen from "./components/HomeScreen.js";
import CoolSideGraph from "./components/CoolSideGraph.js";
import OutSideGraph from "./components/OutSideGraph.js";
import HeatGraph from "./components/HeaterGraph.js";
import Heater_ON_OFF_Graph from "./components/Heater_ON_OFF.js";

// import { useMQTT } from "./components/MQTTService";

const Tab = createMaterialTopTabNavigator();

function App() {
  return (
    <SafeAreaView style={styles.AndroidSafeArea}>
      <NavigationContainer>
        <Tab.Navigator
          initialRouteName="Home"
          screenOptions={{
            tabBarActiveTintColor: "red",
            tabBarInactiveTintColor: "blue",
            tabBarLabelStyle: { fontSize: 16 },
            tabBarIndicatorStyle: { backgroundColor: "red" },
            tabBarStyle: { backgroundColor: "white" },
          }}
        >
          <Tab.Screen name="Home" component={HomeScreen} />
          <Tab.Screen name="Gauges" component={GaugeScreen} />
          <Tab.Screen name="Settings" component={SettingsScreen} />
          <Tab.Screen name="coolSide" component={CoolSideGraph} />
          <Tab.Screen name="outSide" component={OutSideGraph} />
          <Tab.Screen name="heatGraph" component={HeatGraph} />
          <Tab.Screen name="HeaterStatus" component={Heater_ON_OFF_Graph} />
        </Tab.Navigator>
      </NavigationContainer>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  AndroidSafeArea: {
    flex: 1,
    backgroundColor: "white",
    paddingTop: Platform.OS === "android" ? StatusBar.currentHeight : 0,
  },
});
export default App;
