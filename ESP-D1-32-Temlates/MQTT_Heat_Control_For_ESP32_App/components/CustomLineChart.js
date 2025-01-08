import React, { useRef, useEffect, useState } from "react";
import { View, ScrollView, Text } from "react-native";
import { LineChart } from "react-native-gifted-charts";
import { useFocusEffect } from "@react-navigation/native";
import { styles } from "../Styles/styles";

const CustomLineChart = ({ data, GraphTextcolor, curved }) => {
  const scrollViewRef = useRef(null);
  const [lastRenderedDate, setLastRenderedDate] = useState(null); // To track the last rendered date
  const debugMode = true; // Set to false to restore normal behavior

  const scrollToEnd = () => {
    if (scrollViewRef.current) {
      scrollViewRef.current.scrollToEnd({ animated: true });
    }
  };

  useEffect(() => {
    scrollToEnd();
  }, [data]);

  useFocusEffect(
    React.useCallback(() => {
      scrollToEnd();
    }, [])
  );
  const renderXAxisLabel = (value, index) => {
    console.log(" index:", index);
    console.log("renderXAxisLabel - value:", value);
    let dateString = "";
    let timeString = value.label;

    // Check if value.label is a valid string
    if (value.label) {
      console.log("value.label:", value.label);
      // Combine current date with time to create a valid date object
      const today = new Date().toISOString().split("T")[0]; // Get current date in YYYY-MM-DD format
      const combinedDateTime = `${today}T${value.label}`;
      const date = new Date(combinedDateTime);

      if (!isNaN(date)) {
        // dateString = date.toLocaleDateString(); // Format the date
        const currentDate = date.toLocaleDateString(); // Format the current date
        timeString = date.toLocaleTimeString([], {
          hour: "2-digit",
          minute: "2-digit",
        }); // Format the time
        // Render the date only if it's the first occurrence of the day
        //  if (lastRenderedDate !== currentDate) {
        //   dateString = currentDate; // Render the date
        //   setLastRenderedDate(currentDate); // Update the last rendered date
        // Render the date if it's the first data point or the date has changed
        // if (index === 0 || lastRenderedDate !== currentDate) {
        //   dateString = currentDate;
        //   setLastRenderedDate(currentDate); // Update the state with the new date
        // }
        console.log(" index:", index);
        if (debugMode || index === 0 || lastRenderedDate !== currentDate) {
          console.log("renderXAxisLabel - value:", value);
          dateString = currentDate;
          if (!debugMode) {
            setLastRenderedDate(currentDate); // Update the state with the new date
          }
        }
      }
    }

    return (
      <View style={{ alignItems: "center" }}>
        {dateString ? (
          <Text style={{ color: "green", fontSize: 12 }}>{dateString}</Text>
        ) : null}
        <Text style={{ color: "green", fontSize: 19 }}>{timeString}</Text>
      </View>
    );
  };

  return (
    <View style={styles.lineGraphContainer}>
      <ScrollView
        ref={scrollViewRef}
        horizontal
        contentContainerStyle={{ flexGrow: 1 }}
      >
        <LineChart
          data={data}
          color="blue"
          thickness={2}
          width={data.length * 94} // Adjust width based on data length
          height={550}
          initialSpacing={0}
          xAxisLabelComponent={(value, index) => renderXAxisLabel(value, index)} // Pass index here
          // xAxisLabelComponent={renderXAxisLabel}
          yAxisLabelTextStyle={{ color: "red", fontSize: 22 }}
          dataPointsRadius={3}
          textShiftY={-25} // Move data points up by 10 units={20}
          hideYAxisText
          spacing={94}
          textFontSize={26} // font size of the text on the data points
          textColor1={GraphTextcolor}
          showScrollIndicator={true}
          yAxisMinValue={-10} // Ensure y-axis starts from =10
          yAxisMaxValue={40} // Ensure y-axis ends at 30
          curved={curved} // Make the line points curved
        />
      </ScrollView>
    </View>
  );
};
export default CustomLineChart;
