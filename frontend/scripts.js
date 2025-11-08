let currentChart = null;
let currentFile = null;

const getBackendUrl = () => {
    const backend = document.getElementById('backend-select').value;
    return backend === 'cpp' ? 'http://localhost:8080' : 'http://localhost:8081';
};

const updateStats = async () => {
    try {
        const response = await fetch(`${getBackendUrl()}/stats`);
        if (response.ok) {
            const stats = await response.json();
            document.getElementById('stats-text').textContent = `Vectors: ${stats.total_vectors}`;
        }
    } catch (error) {
        console.error('Failed to fetch stats:', error);
    }
};

const showToast = (message, type = 'success') => {
    const container = document.getElementById('toast-container');
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.textContent = message;
    container.appendChild(toast);
    
    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateX(400px)';
        setTimeout(() => toast.remove(), 300);
    }, 3000);
};

const updateCharCounter = () => {
    const text = document.getElementById('resume-text').value;
    document.getElementById('char-count').textContent = text.length;
};

document.getElementById('resume-text').addEventListener('input', updateCharCounter);

// Update stats on load
updateStats();
setInterval(updateStats, 5000); // Update every 5 seconds

// Tab switching
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        const tab = btn.dataset.tab;
        
        // Update buttons
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        
        // Update content
        document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
        document.getElementById(`${tab}-tab`).classList.add('active');
    });
});

// File upload
document.getElementById('file-select-btn').addEventListener('click', () => {
    document.getElementById('file-input').click();
});

document.getElementById('file-input').addEventListener('change', async (e) => {
    const file = e.target.files[0];
    if (!file) return;
    
    currentFile = file;
    const fileInfo = document.getElementById('file-info');
    fileInfo.className = 'file-info has-file';
    fileInfo.innerHTML = `
        <strong>${file.name}</strong><br>
        Size: ${(file.size / 1024).toFixed(2)} KB<br>
        Type: ${file.type || 'Unknown'}
    `;
    
    showToast(`File selected: ${file.name}`, 'success');
});

async function readFileContent(file) {
    const extension = file.name.split('.').pop().toLowerCase();
    
    if (extension === 'txt') {
        return await file.text();
    } else if (extension === 'pdf') {
        try {
            const arrayBuffer = await file.arrayBuffer();
            const pdf = await pdfjsLib.getDocument({data: arrayBuffer}).promise;
            let fullText = '';
            
            for (let i = 1; i <= pdf.numPages; i++) {
                const page = await pdf.getPage(i);
                const textContent = await page.getTextContent();
                const pageText = textContent.items.map(item => item.str).join(' ');
                fullText += pageText + '\n\n';
            }
            
            return fullText;
        } catch (error) {
            showToast(`PDF reading error: ${error.message}`, 'error');
            return null;
        }
    } else if (extension === 'doc' || extension === 'docx') {
        showToast('DOC/DOCX files require server-side processing. Please convert to PDF or TXT first.', 'warning');
        return null;
    } else {
        showToast('Unsupported file type. Please use TXT or PDF files.', 'error');
        return null;
    }
}

// Clear database
document.getElementById('clear-db-btn').addEventListener('click', () => {
    document.getElementById('confirm-modal').style.display = 'flex';
});

document.getElementById('cancel-clear').addEventListener('click', () => {
    document.getElementById('confirm-modal').style.display = 'none';
});

document.getElementById('confirm-clear').addEventListener('click', async () => {
    document.getElementById('confirm-modal').style.display = 'none';
    
    try {
        const response = await fetch(`${getBackendUrl()}/clear`, {
            method: 'POST'
        });
        
        if (response.ok) {
            showToast('Database cleared successfully!', 'success');
            updateStats();
            // Clear results
            document.getElementById('results-section').style.display = 'none';
            document.getElementById('visualization-section').style.display = 'none';
        } else {
            showToast('Failed to clear database', 'error');
        }
    } catch (error) {
        showToast(`Error: ${error.message}`, 'error');
    }
});

document.getElementById('upload-btn').addEventListener('click', async () => {
    let resumeText = '';
    
    // Check which tab is active
    const activeTab = document.querySelector('.tab-content.active');
    if (activeTab.id === 'text-tab') {
        resumeText = document.getElementById('resume-text').value.trim();
        if (!resumeText) {
            showToast('Please enter some text', 'error');
            return;
        }
    } else if (activeTab.id === 'file-tab') {
        if (!currentFile) {
            showToast('Please select a file first', 'error');
            return;
        }
        showToast('Reading file...', 'warning');
        resumeText = await readFileContent(currentFile);
        if (!resumeText) {
            return; // Error already shown in readFileContent
        }
    }

    const btn = document.getElementById('upload-btn');
    btn.disabled = true;
    btn.textContent = 'Uploading...';

    try {
        const response = await fetch(`${getBackendUrl()}/upload_resume`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ resume: resumeText })
        });

        if (response.ok) {
            showToast('Document vectorized and uploaded successfully!', 'success');
            document.getElementById('resume-text').value = '';
            currentFile = null;
            document.getElementById('file-input').value = '';
            document.getElementById('file-info').innerHTML = '';
            document.getElementById('file-info').className = 'file-info';
            updateCharCounter();
            updateStats();
        } else {
            const errorText = await response.text();
            showToast(`Upload failed: ${errorText}`, 'error');
        }
    } catch (error) {
        showToast(`Connection error: ${error.message}`, 'error');
    } finally {
        btn.disabled = false;
        btn.innerHTML = `
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor">
                <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4M17 8l-5-5-5 5M12 3v12" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
            </svg>
            Vectorize & Upload
        `;
    }
});

document.getElementById('query-btn').addEventListener('click', async () => {
    const queryText = document.getElementById('query-text').value.trim();
    const k = parseInt(document.getElementById('k-value').value);
    
    if (!queryText) {
        showToast('Please enter a search query', 'error');
        return;
    }

    const btn = document.getElementById('query-btn');
    btn.disabled = true;
    btn.textContent = 'Searching...';

    try {
        const response = await fetch(`${getBackendUrl()}/query`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ query: queryText, k })
        });

        if (response.ok) {
            const results = await response.json();
            if (results.length === 0) {
                showToast('No results found. Upload some documents first!', 'warning');
            } else {
                displayResults(results);
                visualizeResults(results);
                showToast(`Found ${results.length} results`, 'success');
            }
        } else {
            const errorText = await response.text();
            showToast(`Query failed: ${errorText}`, 'error');
        }
    } catch (error) {
        showToast(`Connection error: ${error.message}`, 'error');
    } finally {
        btn.disabled = false;
        btn.innerHTML = `
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor">
                <circle cx="11" cy="11" r="8" stroke-width="2"/>
                <path d="m21 21-4.35-4.35" stroke-width="2" stroke-linecap="round"/>
            </svg>
            Search
        `;
    }
});

document.getElementById('clear-results').addEventListener('click', () => {
    document.getElementById('results-section').style.display = 'none';
    document.getElementById('visualization-section').style.display = 'none';
    if (currentChart) {
        currentChart.destroy();
        currentChart = null;
    }
});

function displayResults(results) {
    const resultsSection = document.getElementById('results-section');
    const resultsList = document.getElementById('results-list');
    
    resultsSection.style.display = 'block';
    resultsList.innerHTML = '';
    
    results.forEach((result, index) => {
        const item = document.createElement('div');
        item.className = 'result-item';
        item.style.animationDelay = `${index * 0.05}s`;
        
        const text = result.metadata?.text || 'No text available';
        item.innerHTML = `
            <strong>Result #${index + 1} (ID: ${result.id})</strong>
            <div class="distance">Distance: ${result.distance.toFixed(4)}</div>
            <div class="text">${text}</div>
        `;
        resultsList.appendChild(item);
    });
}

function visualizeResults(results) {
    const section = document.getElementById('visualization-section');
    section.style.display = 'block';
    
    if (currentChart) {
        currentChart.destroy();
    }
    
    const ctx = document.getElementById('chart').getContext('2d');
    
    currentChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: results.map((r, i) => `Result #${i + 1}`),
            datasets: [{
                label: 'Similarity Distance',
                data: results.map(r => r.distance),
                backgroundColor: 'rgba(102, 126, 234, 0.6)',
                borderColor: 'rgba(102, 126, 234, 1)',
                borderWidth: 2,
                borderRadius: 8
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            plugins: {
                legend: {
                    labels: {
                        color: '#f8fafc',
                        font: {
                            family: 'Inter',
                            size: 12
                        }
                    }
                },
                tooltip: {
                    backgroundColor: '#1e293b',
                    titleColor: '#f8fafc',
                    bodyColor: '#94a3b8',
                    borderColor: '#334155',
                    borderWidth: 1,
                    padding: 12,
                    displayColors: false,
                    callbacks: {
                        label: (context) => {
                            return `Distance: ${context.parsed.y.toFixed(4)}`;
                        }
                    }
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    grid: {
                        color: '#334155',
                        drawBorder: false
                    },
                    ticks: {
                        color: '#94a3b8',
                        font: {
                            family: 'Inter',
                            size: 11
                        }
                    }
                },
                x: {
                    grid: {
                        display: false
                    },
                    ticks: {
                        color: '#94a3b8',
                        font: {
                            family: 'Inter',
                            size: 11
                        }
                    }
                }
            }
        }
    });
}
