const express = require("express");
const cors = require("cors");
const app = express();
const PORT = process.env.PORT || 3000;
// Middleware to allow web pages (e.g., a browser dashboard) on other origins to call this API
app.use(cors());
// Middleware to parse JSON payloads (makes the data available as req.body)
app.use(express.json());
// Import routes from the routes folder
const apiRoutes = require("./routes");
app.use("/api", apiRoutes); // Prefix all routes with /api
// Home page: a quick check that the server is alive
app.get("/", (req, res) => {
  res.json({
    message: "Activity 5 ESP32 Sensor API",
    readings: "/api/readings",
  });
});
// Any unknown route returns a JSON 404 so Postman and the ESP32 get a clear answer
app.use((req, res) => {
  res.status(404).json({ error: "Route not found" });
});
// Start the server. "0.0.0.0" lets other devices on the same Wi-Fi (the ESP32) connect.
// Vercel imports the app instead of calling listen(), so we skip it there.
if (process.env.VERCEL !== "1") {
  app.listen(PORT, "0.0.0.0", () => {
    console.log(`Server is running on http://localhost:${PORT}`);
  });
}
module.exports = app;
