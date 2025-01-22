// styles.js
import { StyleSheet } from "react-native"; // Use this line only if you are using React Native

export const styles = StyleSheet.create({
  container: {
    flexGrow: 1,
    alignItems: "center",
    marginTop: 2,
    paddingTop: 50,
  },
  heading: {
    fontSize: 24,
    color: "red",
    marginBottom: 15,
    fontStyle: "italic",
    fontFamily: "sans-serif",
    textDecorationLine: "underline",
  },
  timeText: {
    justifyContent: "space-between",
    alignItems: "center",
    marginBottom: 10,
    fontSize: 20,
    marginLeft: 15,
    color: "blue",
  },
  time: {
    // justifyContent: "space-between",
    alignItems: "center",
    left: "7%",
    marginBottom: 10,
    fontSize: 20,
    color: "blue",
  },
  header: {
    fontSize: 20,

    color: "red",
    marginBottom: 10,
    fontStyle: "italic",
    fontFamily: "sans-serif",
    textDecorationLine: "underline",
  },
  TargetTempText: {
    fontSize: 24,
    color: "blue",
    padding: 10,
  },
  tempContainer: {
    marginTop: 30,
    marginBottom: 40,
  },
  timeHeader: {
    fontSize: 20,
    color: "blue",
    marginBottom: 10,
    fontStyle: "italic",
    fontFamily: "sans-serif",
  },
  tempText: {
    fontWeight: "bold",
    color: "#008060",
    fontSize: 20,
  },
  reconnectButton: {
    backgroundColor: "blue",
    padding: 10,
    margin: 10,
    borderRadius: 5,
  },
  reconnectText: {
    color: "white",
    fontSize: 20,
  },
  connectionStatus: {
    fontSize: 20,
    margin: 20,
  },
  pickerContainer: {
    fontSize: 10,
    color: "brown",
    marginBottom: 10,
  },
  temperatureText: {
    // padding: 20,
    marginBottom: 10,
    fontSize: 20,
    color: "blue",
  },
  dataText: {
    // backgroundColor: "#fff",
    color: "blue",
    margintop: 20,
    fontSize: 20,
  },
  dataReset: {
    fontSize: 20,
    marginBottom: 10,
    // backgroundColor: "#fff",
    justifyContent: "center",
    alignItems: "center",
    // margin: 10,
    color: "green",
  },
  reset: {
    justifyContent: "center",

    alignItems: "center",
    padding: 10,
    color: "blue",
    backgroundColor: "#5ff9",
    borderRadius: 25,
    borderWidth: 1,
    borderColor: "red",
    // Add any additional styling you need for the TouchableOpacity here
  },
  homeContainer: {
    flex: 1,
    justifyContent: "center", // Centers children vertically in the container
    alignItems: "center", // Centers children horizontally in the container
  },
  image: {
    width: "100%", // Example width
    height: "100%", // Example height
    justifyContent: "center", // Centers children vertically in the container
    alignItems: "center", // Centers children horizontally in the container
  },
  button: {
    margin: 10,
    width: 250,
    backgroundColor: "rgba(173,216,230,0.80)", // Semi-transparent light bluebackground
    borderWidth: 1,
    padding: 5,
    borderRadius: 5,
  },
  buttonText: {
    alignItems: "center",
    fontSize: 20,
    color: "red",
  },
  graphContainer: {
    flexGrow: 1,
    alignItems: "center",
    marginTop: 50,
    paddingTop: 50,
  },
  lineGraphContainer: {
    flexGrow: 1,
    alignItems: "center",
    marginTop: 5,
    paddingTop: 150,
  },
  homeHeader: {
    fontSize: 24,
    color: "red",
    marginBottom: 15,
    fontStyle: "italic",
    fontFamily: "sans-serif",
    textDecorationLine: "underline",
  },
});
