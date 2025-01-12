//HomeScreen.js
import React from "react";
//import { NavigationContainer } from "@react-navigation/native";
import { SafeAreaView } from "react-native-safe-area-context"; // Import from react-native-safe-area-context
import { ImageBackground, Text, TouchableOpacity } from "react-native";
import { styles } from "../Styles/styles";
import backgroundImage from "../assets/background-image.png";

export default function HomeScreen({ navigation }) {
  return (
    <SafeAreaView style={styles.homeContainer}>
      <ImageBackground source={backgroundImage} style={[styles.image]}>
      <Text style={styles.homeHeader}>MQTT_Heat_Control</Text>
        <TouchableOpacity
          style={styles.button}
          onPress={() => navigation.navigate("Home")}
        >
          <Text style={styles.buttonText}>Go To Home Screen</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={styles.button}
          onPress={() => navigation.navigate("Gauges")}
        >
          <Text style={styles.buttonText}>Go To Gauges Screen</Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={styles.button}
          onPress={() => navigation.navigate("Settings")}
        >
          <Text style={styles.buttonText}>Go To Settings Screen</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={styles.button}
          onPress={() => navigation.navigate("coolSide")}
        >
          <Text style={styles.buttonText}>Go To CoolSide Screen</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={styles.button}
          onPress={() => navigation.navigate("coolSide")}
        >
          <Text style={styles.buttonText}>Go To Heater Screen</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={styles.button}
          onPress={() => navigation.navigate("outSide")}
        >
          <Text style={styles.buttonText}>Go To OutSide Screen</Text>
        </TouchableOpacity>
      </ImageBackground>
    </SafeAreaView>
  );
}
