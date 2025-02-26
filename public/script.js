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

        //
        db.ref('temperature').limitToLast(30).once('value').then(function(snapshot) {
            const data = snapshot.val();  // Hent data fra Firebase
            const xValues = [];  // Tidspunkt (eller indekser)
            const yValues = [];  // Temperaturdata

            let i = 0;  // Bruk en teller for x-verdiene
            for (const key in data) {
                if (data.hasOwnProperty(key)) {
                    xValues.push(i);  // Her kan du bruke tidsstempel eller annet ID som x-verdi
                    yValues.push(data[key].temperature);  // Temperaturverdiene fra Firebase
                    i++;
                }
            }

            // Vis grafen
            plotGraph(xValues, yValues);
        });

        // Funksjon for å vise grafen med Plotly.js
        function plotGraph(xValues, yValues) {
            const data = [{
                x: xValues,
                y: yValues,
                mode: "lines",
                type: "scatter"
            }];

            const layout = {
                xaxis: {title: "Tid (dager)", range: [0, xValues.length]},  // Juster tidsskalaen
                yaxis: {title: "Temperatur (°C)", range: [20, 50]},  // Juster temperaturintervallet
                title: "Temperaturmålinger"
            };

            Plotly.newPlot("temperatureChart", data, layout);
        }
        //
        });

    } catch (error) {
        console.error("Firebase initialization error:", error);
        document.getElementById("dataContainer").textContent = "Error loading Firebase";
    }
});
