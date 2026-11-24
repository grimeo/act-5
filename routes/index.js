const express = require("express");
const router = express.Router();

// Import route files
const readingRoutes = require("./readingRoutes");
const deviceRoutes = require("./deviceRoutes");

// Register API routes
router.use("/readings", readingRoutes);
router.use("/devices", deviceRoutes);

module.exports = router;
