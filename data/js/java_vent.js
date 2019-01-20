     google.charts.load('current', {
        'packages': ['gauge']
      });
      google.charts.setOnLoadCallback(drawChart); //  https://developers.google.com/chart/interactive/docs/gallery/gauge

      function drawChart() {    // Fonction principale : 

        // Create and populate the data table.
        var dataGaugeVent = google.visualization.arrayToDataTable([
          ['Label', 'Value'],
          ['Vent', 0],
        ]);

        // formatage avec un suffixe et # pour arrondir
        var formatter = new google.visualization.NumberFormat({
          suffix: ' Km/h',
          pattern: '#'
        });

        // définition des options documentée ici https://developers.google.com/chart/interactive/docs/gallery/gauge
        var options = {
          width: 800, height: 720,
          greenFrom:0, greenTo: 60,   // zones de couleurs
          yellowFrom:60, yellowTo: 80,
          redFrom: 80, redTo: 100,
          majorTicks: ['0', 10, 20, 0, 0, 50, 0, 0, 80, 90, 100], // graduations
          minorTicks: 5, // graduations
          max: 100, min: 0,
          animation: {
            duration: 100,  // vitesse de l'aiguille
            easing: 'linear',   // comportement de l'aiguille
          }
        };

        // init du graphisme
        var GaugeVent = new google.visualization.Gauge(document.getElementById('chart_div_vent'));

        // tracé du graphisme
        GaugeVent.draw(dataGaugeVent, options);
        updateGauge();  // exécute la fontion
        setInterval(updateGauge, 100); // appelle les données de la fonction et définit la vitesse de rafraichissement

        function updateGauge() {     // fonction de mise raffraichissement des données réccupérée dans le fichier json https://www.w3schools.com/jquery/ajax_getjson.asp

          $.getJSON('mesures.json',

            function(data) {

              $.each(data,

                function(nom, valeur) {

                  dataGaugeVent.setValue(0, 1, valeur); // premier nb pour le N° de la gauge si plusieurs, puis 0 pour modifier le champ label (sinon:1 pour le champ Value), puis la valeur extraite du json
                  GaugeVent.draw(dataGaugeVent, options); // tracé du graphisme
                  formatter.format(dataGaugeVent, 1); // applique le formattage à la seconde collonne du tableau (1) qui correspond à la valeur

                });
            });
        }
      }
