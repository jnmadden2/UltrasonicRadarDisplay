String angle="";
String distance="";
String data="";
String noObject;
float pixsDistance;
int iAngle, iDistance;
int index1=0;
PFont font;

String[] dataEntries;  // Array to hold each data entry split by '.'
int entryIndex = 0;    // Track the current entry being processed

final int MAX_POINTS = 10;
final int MAX_AGE = 10;     // Maximum age for fading effect

ArrayList<DataPoint> dataPoints = new ArrayList<DataPoint>(); // List to store up to 10 points

class DataPoint {
  float x, y;
  int age;

  DataPoint(float x, float y) {
    this.x = x;
    this.y = y;
    this.age = 0;
  }
}

void setup() {
  size (1920, 1100); // 
  fullScreen();
  smooth();
  font= loadFont("SegoeUI-Semilight-48.vlw");
  
  // Load the entire file as a single string
  String rawData = join(loadStrings("sampleData.txt"), " "); // Load all lines and join into one string

  // Split the data by the '.' character
  dataEntries = rawData.split("\\."); // Use "\\." to escape the dot character

  // Remove any empty strings from the array (could happen if the last entry had a trailing period)
  dataEntries = removeEmptyEntries(dataEntries);

  if (dataEntries.length == 0) {
    println("No data found.");
    exit();
  }
}

void draw() {
  textFont(font);
  noStroke();
  fill(0, 4);
  rect(0, 0, width, height-height*0.065);
  fill(0,0,255);
  
  if (entryIndex < dataEntries.length) {
    // Process the current entry
    processData(dataEntries[entryIndex].trim());
    
    
    // Draw based on the processed data
    //drawData(iAngle, iDistance);
    fill(0);
    rect(0, 0, width, height-height*0.065);
    fill(0, 4);
    rect(0, 0, width, height-height*0.065);
    drawRadar();
    drawText();
    drawObject();
    drawCurrentValues();
    println("Data points: " + dataPoints.size());    
    // Move to the next entry for the next frame
    delay(1000);  // Wait for 2000 milliseconds (2 second) this is to simulate waiting for data
    entryIndex++;  
  } else {
    println("End of data reached.");
    noLoop();  // Stop drawing once all entries are processed
  }
  

}

void drawRadar() {
  pushMatrix();
  translate(width / 2, height - height * 0.074); // Set the radar center position
  noFill();
  strokeWeight(2);
  stroke(255, 255, 0); // Grid color

  // Calculate spacing for each arc
  int maxDistance = 100; // Maximum distance in cm for radar
  int increment = 10;    // Distance increment per arc
  int arcCount = maxDistance / increment; // Total number of arcs

  float maxRadius = width * 0.9; // Maximum radius for the outermost arc
  float radiusIncrement = maxRadius / arcCount; // Radius increment per arc

  // Draw arcs for each 10 cm increment
  for (int i = 1; i <= arcCount; i++) {
    float radius = radiusIncrement * i;
    arc(0, 0, radius, radius, PI, TWO_PI);
  }

  // Draw angle lines
  line(-width / 2, 0, width / 2, 0); // Horizontal line
  for (int angle = 30; angle < 180; angle += 30) {
    float x = -width / 2 * cos(radians(angle));
    float y = -width / 2 * sin(radians(angle));
    line(0, 0, x, y);
  }

  popMatrix();
}

void drawText() {
  pushMatrix();

  // Determine if the object is in range or out of range
  if (iDistance > 100) {
    noObject = "Out of Range";
  } else {
    noObject = "In Range";
  }

  // Draw background for information text at the bottom of the screen
  fill(0); // Solid black background
  noStroke();
  rect(0, height - height * 0.0648, width, height * 0.0648);  // Covers only the text area

  // Set color and text size for object information text
  fill(255, 10, 10); // Red color for visibility
  textSize(40);
  textAlign(CENTER, CENTER);  // Center-align for consistent alignment

  // Display the object status, angle, and distance, centered in the designated area
  //text("Object: " + noObject, width * 0.2, height - height * 0.0324); // Left-side text
  //text("Angle: " + iAngle + "°", width * 0.5, height - height * 0.0324); // Centered text
  //text("Distance: " + iDistance + " cm", width * 0.8, height - height * 0.0324); // Right-side text

  // Set up for radar labels
  textSize(25);
  fill(255, 10, 10);  // Red color for radar labels
  float radarCenterX = width / 2;
  float radarCenterY = height - height * 0.074;

  // Draw distance labels along each arc, centered and moved down slightly
  for (int i = 1; i <= 10; i++) {
    int distance = i * 10;
    float radius = map(distance, 0, 100, 0, width / 2 - width * 0.0625); // Scales to fit radar size

    // Left-side label at 180 degrees, slightly below the arc line
    pushMatrix();
    translate(radarCenterX - radius, radarCenterY + 10); // Moves down by 10 pixels
    textAlign(CENTER, TOP); // Centered alignment for "cm" markers
    text(distance + " cm", 0, 0);
    popMatrix();

    // Right-side label at 0 degrees, slightly below the arc line
    pushMatrix();
    translate(radarCenterX + radius, radarCenterY + 10); // Moves down by 10 pixels
    textAlign(CENTER, TOP); // Centered alignment
    text(distance + " cm", 0, 0);
    popMatrix();
  }

  // Draw angle markers at 30-degree intervals
  int[] angles = {30, 60, 90, 120, 150};
  float lineLength = width / 2; // Match line length in drawRadar

  for (int angle : angles) {
    float x = radarCenterX + lineLength * cos(radians(angle));
    float y = radarCenterY - lineLength * sin(radians(angle));

    pushMatrix();
    translate(x, y);
    rotate(radians(-angle + 90)); // Rotate text outward
    textAlign(CENTER, CENTER); // Center-align angle labels
    text(angle + "°", 0, 0);
    popMatrix();
  }

  popMatrix();
}


void drawObjectOld() {
  // Define radar center position
  float radarCenterX = width / 2;
  float radarCenterY = height - height * 0.074;

  // Maximum radius of the radar display
  float maxRadius = width * 0.45;  // Adjust this value to match the radar's outermost arc

  // Limit the distance to the radar's maximum range
  int maxDistance = 1000;
  int clampedDistance = constrain(iDistance, 0, maxDistance);

  // Map the clamped distance to a radius within the radar's maximum range
  float radius = map(clampedDistance, 0, maxDistance, 0, maxRadius);

  // Convert polar coordinates (distance, angle) to Cartesian coordinates
  float angleInRadians = radians(iAngle); // Convert iAngle from degrees to radians
  float x = radarCenterX + radius * cos(angleInRadians);
  float y = radarCenterY - radius * sin(angleInRadians); // Inverting y to align with Processing's coordinate system

  // Draw the point on the radar
  fill(255, 0, 0); // Set color to red for visibility
  noStroke();
  ellipse(x, y, 8, 8); // Draw the object as a small circle or point
}

void drawObjectTest() {
  // Define radar center position
  float radarCenterX = width / 2;
  float radarCenterY = height - height * 0.074;

  // Maximum radius of the radar display
  float maxRadius = width * 0.45;  // Adjust this value to match the radar's outermost arc

  for (DataPoint point : dataPoints) {
    // Calculate opacity based on the point's age
    float opacity = map(point.age, 0, MAX_AGE, 255, 0);
    fill(255, 0, 0, opacity);  // Set color to red with opacity based on age
    noStroke();

    // Draw the point on the radar
    ellipse(point.x, point.y, 8, 8); // Draw the object as a small circle or point

    // Increment the age of the point for the fading effect
    point.age++;
  }
}

void drawObject() {
  // Define radar center position
  float radarCenterX = width / 2;
  float radarCenterY = height - height * 0.074;

  // Maximum radius of the radar display
  float maxRadius = width * 0.45;  // Adjust this value to match the radar's outermost arc

  // Iterate in reverse to safely remove aged-out points
  for (int i = dataPoints.size() - 1; i >= 0; i--) {
    DataPoint point = dataPoints.get(i);

    // Calculate opacity based on the point's age
    float opacity = map(point.age, 0, MAX_AGE, 255,120);
    fill(255, 0, 0, opacity);  // Set color to red with opacity based on age
    noStroke();

    // Draw the point on the radar
    ellipse(point.x, point.y, 8, 8); // Draw the object as a small circle or point

    // Increment the age of the point
    point.age++;

    // Remove point if it exceeds MAX_AGE
    if (point.age > MAX_AGE) {
      dataPoints.remove(i);
    }
  }
}


void processData(String data) {
  // Find the index of the comma separator
  int index1 = data.indexOf(",");
  if (index1 == -1) {
    println("Invalid data format: " + data);
    return;
  }

  // Extract angle and distance as substrings based on comma position
  distance = data.substring(0, index1);
  angle = data.substring(index1 + 1);

  // Convert strings to integers
  iAngle = int(angle);
  iDistance = int(distance);

  // Print or use the values
  println("Angle: " + iAngle + ", Distance: " + iDistance);
  
   // Convert polar coordinates (angle, distance) to Cartesian coordinates (x, y)
  float radarCenterX = width / 2;
  float radarCenterY = height - height * 0.074;
  float maxRadius = width * 0.45; // Match the radar's outermost arc
  int maxDistance = 1000;
  int clampedDistance = constrain(iDistance, 0, maxDistance);
  float radius = map(clampedDistance, 0, maxDistance, 0, maxRadius);

  // Calculate x and y based on angle and distance
  float angleInRadians = radians(iAngle);
  float x = radarCenterX + radius * cos(angleInRadians);
  float y = radarCenterY - radius * sin(angleInRadians);
  
  // Add the new data point
  dataPoints.add(new DataPoint(x, y));
  if (dataPoints.size() > MAX_POINTS) {
    dataPoints.remove(0);
  }
}

String[] removeEmptyEntries(String[] entries) {
  // Create a new list to hold valid entries
  ArrayList<String> validEntries = new ArrayList<String>();
  
  // Loop through the entries and add non-empty ones to the list
  for (int i = 0; i < entries.length; i++) {
    if (!entries[i].trim().isEmpty()) {
      validEntries.add(entries[i].trim()); // Trim the entry before adding
    }
  }
  
  // Convert the list back to an array
  return validEntries.toArray(new String[validEntries.size()]);
}

void drawCurrentValues() {
  // Draw the current angle and distance in the top right corner
  fill(255, 255, 255);  // White color for text
  textSize(32);         // Set text size
  textAlign(RIGHT, TOP); // Align text to the top-right corner

  // Display the current distance and angle
  text("Angle: " + iAngle + "°", width - 20, 20);  // Adjust X and Y for positioning
  text("Distance: " + iDistance + " mm", width - 20, 60);  // Adjust Y for distance
}
