document.getElementById('upload-btn').addEventListener('click', async () => {
    const resumeText = document.getElementById('resume-text').value;
    if (!resumeText) return alert('Please enter resume text');

    const response = await fetch('http://localhost:8080/upload_resume', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ resume: resumeText })
    });

    if (response.ok) {
        alert('Resume uploaded successfully');
    } else {
        alert('Error uploading resume');
    }
});

document.getElementById('query-btn').addEventListener('click', async () => {
    const queryText = document.getElementById('query-text').value;
    const k = document.getElementById('k-value').value;
    if (!queryText) return alert('Please enter a query');

    const response = await fetch('http://localhost:8080/query', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ query: queryText, k: parseInt(k) })
    });

    if (response.ok) {
        const results = await response.json();
        displayResults(results);
        visualizeResults(results);
    } else {
        alert('Error querying');
    }
});

function displayResults(results) {
    const resultsList = document.getElementById('results-list');
    resultsList.innerHTML = '';
    results.forEach(result => {
        const item = document.createElement('div');
        item.className = 'result-item';
        item.innerHTML = `<strong>ID: ${result.id}</strong><br>Distance: ${result.distance.toFixed(4)}<br>Text: ${result.metadata.text}`;
        resultsList.appendChild(item);
    });
}

function visualizeResults(results) {
    const ctx = document.getElementById('chart').getContext('2d');
    const data = {
        labels: results.map(r => `ID ${r.id}`),
        datasets: [{
            label: 'Similarity Distance',
            data: results.map(r => r.distance),
            backgroundColor: 'rgba(75, 192, 192, 0.2)',
            borderColor: 'rgba(75, 192, 192, 1)',
            borderWidth: 1
        }]
    };
    new Chart(ctx, {
        type: 'bar',
        data: data,
        options: {
            scales: {
                y: {
                    beginAtZero: true
                }
            }
        }
    });
}
