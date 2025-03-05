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
        plotGraph(xValues, yValues, "temperatureChart", "Temperaturmålinger", "Tid(dager)", "Temperatur(°C)",[10,40]);
    });
    
    // Temperatur slutter her!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Vekt Begynner her!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Henter vekt fra siste 30 dager
    const vektRef = db.ref("sensor/temperature").limitToLast(60); // endre denne "sensor/temperture" til "sensor/vekt"
    vektRef.once("value").then((snapshot) => {
        
        const data = snapshot.val();
        if (!data) return;

        const xValues = []; // Tidspunkt
        const yValues = []; // vekt

        let i = 0;
        for (const key in data) {
            if (data.hasOwnProperty(key)) {
                xValues.push(i);
                yValues.push(data[key]); // Antar at verdien er direkte vekten
                i++;
            }
        }

    // Vis grafen
    plotGraph(xValues, yValues, "weightChart", "Vektmålinger", "Tid (dager)", "Vekt (kg)", [0, 100]);
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
        xaxis: {title: xTitle, range: [0, xValues.length]},  // Juster tidsskalaen
        yaxis: {title: yTitle, range: [10, 40]},  // Juster vektintervallet
        title: title
    };

    Plotly.newPlot("weightChart", data, layout);
}

    } catch (error) {
        console.error("Firebase initialization error:", error);
        document.getElementById("dataContainer").textContent = "Error loading Firebase";
    }
});
