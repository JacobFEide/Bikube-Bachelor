document.addEventListener('DOMContentLoaded', function () {
    try {
        // Initialize Firebase
        const app = firebase.app();
        const db = firebase.database();

        // Reference to the 'previousTemperature' under 'sensor'
        const dataRef = db.ref("sensor/previoustemp");

        // Fetch and display data
        dataRef.on("value", (snapshot) => {
            const previoustemp = snapshot.val();
            document.getElementById("dataContainer").textContent = previoustemp !== null ? 
                `Real Time Temperature: ${previoustemp}°CA` : "No data available";
        });

    } catch (error) {
        console.error("Firebase initialization error:", error);
        document.getElementById("dataContainer").textContent = "Error loading Firebase";
    }
});
