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
            
            const tempdata = snapshot.val();
            if (!tempdata) return;

            const xValues = []; // Tidspunkt
            const yValues = []; // Temperatur

            let i = 0;
            for (const key in data) {
                if (tempdata.hasOwnProperty(key)) {
                    xValues.push(i);
                    yValues.push(data[key]); // Antar at verdien er direkte temperaturen
                    i++;
                }
            }

        // Vis grafen
        plotGraph(xValues, yValues);
    });

    // Funksjon for å vise grafen med Plotly.js
    function plotGraph(xValues, yValues) {
        const vistempdata = [{
            x: xValues,
            y: yValues,
            mode: "lines",
            type: "scatter"
        }];

        const layout = {
            xaxis: {title: "Tid (dager)", range: [0, xValues.length]},  // Juster tidsskalaen
            yaxis: {title: "Temperatur (°C)", range: [10, 40]},  // Juster temperaturintervallet
            title: "Temperaturmålinger"
        };

        Plotly.newPlot("temperatureChart", vistempdata, layout);
    }
    // Temperatur slutter her!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Vekt Begynner her!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Henter vekt fra siste 30 dager
    const vektRef = db.ref("sensor/temperature").limitToLast(60); // endre denne "sensor/temperture" til "sensor/vekt"
    vektRef.once("value").then((snapshot) => {
        
        const vektdata = snapshot.val();
        if (!vektdata) return;

        const xValues = []; // Tidspunkt
        const yValues = []; // vekt

        let i = 0;
        for (const key in vektdata) {
            if (vektdata.hasOwnProperty(key)) {
                xValues.push(i);
                yValues.push(vektdata[key]); // Antar at verdien er direkte vekten
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
        yaxis: {title: "Vekt (kg)", range: [10, 40]},  // Juster vektintervallet
        title: "Vektmålinger"
    };

    Plotly.newPlot("weightChart", vektdata, layout);
}

    } catch (error) {
        console.error("Firebase initialization error:", error);
        document.getElementById("dataContainer").textContent = "Error loading Firebase";
    }
});
