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
                `Siste måling: ${previoustemp}°C` : "No data available";
        });
        // Henter temp fra siste 30 dager
        const tempRef = db.ref("sensor/temperature").limitToLast(60);
        tempRef.once("value").then((snapshot) => {
            
            const data = snapshot.val();
            if (!data) return;

            const xValues = []; // Tidspunkt
            const yValues = []; // Temperatur

            let i = 0;
            for (const key in data) {
                if (data.hasOwnProperty(key)) {
                    xValues.push(i);
                    yValues.push(data[key]); // Antar at verdien er direkte temperaturen
                    i++;
                }
            }

        // Vis grafen
        plotGraph(xValues, yValues, "temperatureChart", "Temperaturmålinger", "Temperatur (°C)");
    });
        // Hent vektdata fra de siste 30 dagene
        const weightRef = db.ref("sensor/weight").limitToLast(60); // Endre til riktig database-sti
        weightRef.once("value").then((snapshot) => {
            const data = snapshot.val();
            if (!data) return;

            const xValues = [];
            const yValues = [];

            let i = 0;
            for (const key in data) {
                if (data.hasOwnProperty(key)) {
                    xValues.push(i);
                    yValues.push(data[key]);
                    i++;
                }
            }

        // Vis vektgrafen
        plotGraph(xValues, yValues, "weightChart", "Vektmålinger", "Vekt (kg)");
    });

    // Funksjon for å vise grafen med Plotly.js
    function plotGraph(xValues, yValues, chartId, title, yAxisTitle) {
        const data = [{
            x: xValues,
            y: yValues,
            mode: "lines",
            type: "scatter"
        }];

        const layout = {
            xaxis: {title: "Tid (dager)", range: [0, xValues.length]},  // Juster tidsskalaen
            yaxis: {title: yAxisTitle},  // Juster intervallet
            title: title
        };

        Plotly.newPlot(chartId, data, layout);
    }
    //

    } catch (error) {
        console.error("Firebase initialization error:", error);
        document.getElementById("dataContainer").textContent = "Error loading Firebase";
    }
});

