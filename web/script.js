// Face Enhancer Web UI JavaScript

let currentImage = null;
let enhancedImage = null;

// Initialize the application
document.addEventListener('DOMContentLoaded', function() {
    initializeEventListeners();
    initializeSliders();
});

function initializeEventListeners() {
    const fileUploadArea = document.getElementById('fileUploadArea');
    const imageInput = document.getElementById('imageInput');

    // File upload click handler
    fileUploadArea.addEventListener('click', () => {
        imageInput.click();
    });

    // Drag and drop handlers
    fileUploadArea.addEventListener('dragover', handleDragOver);
    fileUploadArea.addEventListener('dragleave', handleDragLeave);
    fileUploadArea.addEventListener('drop', handleDrop);

    // File input change handler
    imageInput.addEventListener('change', handleFileSelect);

    // Prevent default drag behaviors
    ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
        fileUploadArea.addEventListener(eventName, preventDefaults, false);
        document.body.addEventListener(eventName, preventDefaults, false);
    });
}

function initializeSliders() {
    const sliders = [
        { id: 'strengthSlider', valueId: 'strengthValue' },
        { id: 'noiseSlider', valueId: 'noiseValue' },
        { id: 'edgeSlider', valueId: 'edgeValue' }
    ];

    sliders.forEach(slider => {
        const element = document.getElementById(slider.id);
        const valueElement = document.getElementById(slider.valueId);
        
        element.addEventListener('input', function() {
            valueElement.textContent = this.value;
        });
    });
}

function preventDefaults(e) {
    e.preventDefault();
    e.stopPropagation();
}

function handleDragOver(e) {
    document.getElementById('fileUploadArea').classList.add('dragover');
}

function handleDragLeave(e) {
    document.getElementById('fileUploadArea').classList.remove('dragover');
}

function handleDrop(e) {
    document.getElementById('fileUploadArea').classList.remove('dragover');
    const files = e.dataTransfer.files;
    if (files.length > 0) {
        handleFile(files[0]);
    }
}

function handleFileSelect(e) {
    const file = e.target.files[0];
    if (file) {
        handleFile(file);
    }
}

function handleFile(file) {
    // Validate file type
    if (!file.type.startsWith('image/')) {
        showError('Please select a valid image file (JPG, PNG, BMP, TIFF)');
        return;
    }

    // Validate file size (max 50MB)
    if (file.size > 50 * 1024 * 1024) {
        showError('File size too large. Please select an image smaller than 50MB.');
        return;
    }

    const reader = new FileReader();
    reader.onload = function(e) {
        currentImage = e.target.result;
        displayImage(currentImage, file.name, file.size);
        enableEnhanceButton();
    };
    reader.readAsDataURL(file);
}

function displayImage(imageSrc, fileName, fileSize) {
    const previewContainer = document.getElementById('imagePreview');
    const previewImage = document.getElementById('previewImage');
    const imageInfo = document.getElementById('imageInfo');

    previewImage.src = imageSrc;
    
    // Format file size
    const sizeInMB = (fileSize / (1024 * 1024)).toFixed(2);
    imageInfo.textContent = `${fileName} (${sizeInMB} MB)`;
    
    previewContainer.style.display = 'block';
    
    // Hide upload area
    document.getElementById('fileUploadArea').style.display = 'none';
}

function clearImage() {
    currentImage = null;
    enhancedImage = null;
    
    // Reset UI
    document.getElementById('imagePreview').style.display = 'none';
    document.getElementById('fileUploadArea').style.display = 'block';
    document.getElementById('resultsSection').style.display = 'none';
    document.getElementById('imageInput').value = '';
    
    disableEnhanceButton();
}

function enableEnhanceButton() {
    const enhanceBtn = document.getElementById('enhanceBtn');
    enhanceBtn.disabled = false;
}

function disableEnhanceButton() {
    const enhanceBtn = document.getElementById('enhanceBtn');
    enhanceBtn.disabled = true;
}

function getEnhancementParameters() {
    return {
        strength: parseFloat(document.getElementById('strengthSlider').value),
        noiseReduction: parseFloat(document.getElementById('noiseSlider').value),
        edgeEnhancement: parseFloat(document.getElementById('edgeSlider').value),
        superResolution: document.getElementById('superResolution').checked,
        qualityMetrics: document.getElementById('qualityMetrics').checked
    };
}

async function enhanceImage() {
    if (!currentImage) {
        showError('Please select an image first');
        return;
    }

    const enhanceBtn = document.getElementById('enhanceBtn');
    const btnText = enhanceBtn.querySelector('.btn-text');
    const loadingSpinner = enhanceBtn.querySelector('.loading-spinner');
    const processingInfo = document.getElementById('processingInfo');

    // Start loading state
    enhanceBtn.disabled = true;
    btnText.textContent = 'Enhancing...';
    loadingSpinner.style.display = 'inline-block';
    processingInfo.style.display = 'block';

    // Simulate progress
    simulateProgress();

    try {
        const parameters = getEnhancementParameters();
        
        // For simulation mode, create enhanced version locally
        const result = await simulateImageEnhancement(currentImage, parameters);
        
        // Display results
        displayResults(result);
        
        // Show simulation mode message
        showInfo('Note: Using simulation mode. Build the C++ executable for real enhancement.');
        
    } catch (error) {
        console.error('Enhancement error:', error);
        showError('Enhancement failed: ' + error.message);
    } finally {
        // Reset button state
        enhanceBtn.disabled = false;
        btnText.textContent = 'Enhance Image';
        loadingSpinner.style.display = 'none';
        processingInfo.style.display = 'none';
    }
}

async function simulateImageEnhancement(imageSrc, parameters) {
    // Simulate processing time
    await new Promise(resolve => setTimeout(resolve, 4000));
    
    // Create a canvas to simulate enhancement
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    const img = new Image();
    
    return new Promise((resolve) => {
        img.onload = function() {
            canvas.width = img.width;
            canvas.height = img.height;
            
            // Draw original image
            ctx.drawImage(img, 0, 0);
            
            // Apply simulated enhancement effects
            applyEnhancementFilter(ctx, canvas.width, canvas.height, parameters);
            
            const enhancedDataUrl = canvas.toDataURL('image/jpeg', 0.95);
            
            // Generate simulated metrics
            const metrics = generateSimulatedMetrics(parameters);
            
            resolve({
                originalImage: imageSrc,
                enhancedImage: enhancedDataUrl,
                metrics: metrics,
                processingTime: 3.2,
                method: 'simulation'
            });
        };
        
        img.crossOrigin = 'anonymous';
        img.src = imageSrc;
    });
}

function applyEnhancementFilter(ctx, width, height, parameters) {
    // Get image data
    const imageData = ctx.getImageData(0, 0, width, height);
    const data = imageData.data;
    
    // Apply simulated sharpening effect
    const sharpness = parameters.strength || 1.5;
    const contrast = 1 + (sharpness - 1) * 0.2;
    const brightness = 1 + (sharpness - 1) * 0.1;
    
    for (let i = 0; i < data.length; i += 4) {
        // Apply contrast and brightness
        data[i] = Math.min(255, Math.max(0, (data[i] - 128) * contrast + 128 + brightness * 10));     // Red
        data[i + 1] = Math.min(255, Math.max(0, (data[i + 1] - 128) * contrast + 128 + brightness * 10)); // Green
        data[i + 2] = Math.min(255, Math.max(0, (data[i + 2] - 128) * contrast + 128 + brightness * 10)); // Blue
    }
    
    // Apply the modified image data
    ctx.putImageData(imageData, 0, 0);
    
    // Apply additional CSS filters for enhancement simulation
    if (parameters.edgeEnhancement > 1) {
        ctx.filter = `contrast(${100 + parameters.edgeEnhancement * 10}%) brightness(${100 + parameters.edgeEnhancement * 5}%)`;
        ctx.drawImage(ctx.canvas, 0, 0);
        ctx.filter = 'none';
    }
}

function generateSimulatedMetrics(parameters) {
    // Generate realistic-looking metrics based on parameters
    const basePSNR = 25 + (parameters.strength * 3) + (parameters.noiseReduction * 0.5);
    const baseSSIM = 0.75 + (parameters.strength * 0.08) + (parameters.edgeEnhancement * 0.05);
    const baseSharpness = 1.2 + (parameters.strength * 0.4) + (parameters.edgeEnhancement * 0.3);
    
    // Add some realistic variation
    const variation = 0.1;
    const psnr = basePSNR + (Math.random() - 0.5) * variation * 10;
    const ssim = Math.min(0.999, baseSSIM + (Math.random() - 0.5) * variation);
    const sharpness = baseSharpness + (Math.random() - 0.5) * variation;
    
    return {
        psnr: Math.max(20, psnr).toFixed(2),
        ssim: Math.max(0.5, ssim).toFixed(3),
        sharpness: Math.max(1.0, sharpness).toFixed(2)
    };
}

function dataURLToBlob(dataURL) {
    return new Promise((resolve) => {
        const arr = dataURL.split(',');
        const mime = arr[0].match(/:(.*?);/)[1];
        const bstr = atob(arr[1]);
        let n = bstr.length;
        const u8arr = new Uint8Array(n);
        
        while (n--) {
            u8arr[n] = bstr.charCodeAt(n);
        }
        
        resolve(new Blob([u8arr], { type: mime }));
    });
}

function simulateProgress() {
    const progressFill = document.getElementById('progressFill');
    const processingText = document.getElementById('processingText');
    
    const steps = [
        { progress: 20, text: 'Analyzing image...' },
        { progress: 40, text: 'Detecting faces...' },
        { progress: 60, text: 'Reducing noise...' },
        { progress: 80, text: 'Enhancing details...' },
        { progress: 100, text: 'Finalizing...' }
    ];
    
    let currentStep = 0;
    
    const updateProgress = () => {
        if (currentStep < steps.length) {
            const step = steps[currentStep];
            progressFill.style.width = step.progress + '%';
            processingText.textContent = step.text;
            currentStep++;
            
            setTimeout(updateProgress, 800);
        }
    };
    
    updateProgress();
}

async function simulateImageEnhancement(imageSrc, parameters) {
    // Simulate processing time
    await new Promise(resolve => setTimeout(resolve, 4000));
    
    // In a real implementation, this would call the C++ backend
    // For demo purposes, we'll create a simulated enhanced version
    
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    const img = new Image();
    
    return new Promise((resolve) => {
        img.onload = function() {
            canvas.width = img.width;
            canvas.height = img.height;
            
            // Draw original image
            ctx.drawImage(img, 0, 0);
            
            // Apply simulated enhancement effects
            applyEnhancementFilter(ctx, canvas.width, canvas.height, parameters);
            
            const enhancedDataUrl = canvas.toDataURL('image/jpeg', 0.95);
            
            // Generate simulated metrics
            const metrics = generateSimulatedMetrics(parameters);
            
            resolve({
                originalImage: imageSrc,
                enhancedImage: enhancedDataUrl,
                metrics: metrics,
                processingTime: 3.2
            });
        };
        
        img.src = imageSrc;
    });
}

function applyEnhancementFilter(ctx, width, height, parameters) {
    // Get image data
    const imageData = ctx.getImageData(0, 0, width, height);
    const data = imageData.data;
    
    // Apply simulated sharpening effect
    const sharpness = parameters.strength;
    for (let i = 0; i < data.length; i += 4) {
        // Increase contrast slightly
        data[i] = Math.min(255, data[i] * (1 + sharpness * 0.1));     // Red
        data[i + 1] = Math.min(255, data[i + 1] * (1 + sharpness * 0.1)); // Green
        data[i + 2] = Math.min(255, data[i + 2] * (1 + sharpness * 0.1)); // Blue
    }
    
    // Apply the modified image data
    ctx.putImageData(imageData, 0, 0);
    
    // Apply additional filters
    ctx.filter = `contrast(${110 + parameters.strength * 10}%) brightness(${105 + parameters.strength * 5}%)`;
    ctx.drawImage(ctx.canvas, 0, 0);
    ctx.filter = 'none';
}

function generateSimulatedMetrics(parameters) {
    // Generate realistic-looking metrics based on parameters
    const basePSNR = 25 + (parameters.strength * 3) + (parameters.noiseReduction * 0.5);
    const baseSSIM = 0.75 + (parameters.strength * 0.08) + (parameters.edgeEnhancement * 0.05);
    const baseSharpness = 1.2 + (parameters.strength * 0.4) + (parameters.edgeEnhancement * 0.3);
    
    return {
        psnr: basePSNR.toFixed(2),
        ssim: baseSSIM.toFixed(3),
        sharpness: baseSharpness.toFixed(2)
    };
}

function showInfo(message) {
    // Create a temporary info notification
    const notification = document.createElement('div');
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        color: white;
        padding: 15px 20px;
        border-radius: 8px;
        box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
        z-index: 1001;
        font-weight: 500;
        display: flex;
        align-items: center;
        gap: 10px;
        max-width: 400px;
    `;
    
    notification.innerHTML = `
        <i class="fas fa-info-circle"></i>
        ${message}
    `;
    
    document.body.appendChild(notification);
    
    // Remove after 5 seconds
    setTimeout(() => {
        notification.remove();
    }, 5000);
}

function displayResults(result) {
    enhancedImage = result.enhancedImage;
    
    // Show results section
    document.getElementById('resultsSection').style.display = 'block';
    
    // Display before/after images
    document.getElementById('beforeImage').src = result.originalImage;
    document.getElementById('afterImage').src = result.enhancedImage;
    
    // Display metrics if enabled
    if (getEnhancementParameters().qualityMetrics) {
        displayMetrics(result.metrics, result.processingTime, result.method);
    }
    
    // Scroll to results
    document.getElementById('resultsSection').scrollIntoView({ 
        behavior: 'smooth',
        block: 'start'
    });
}

function displayMetrics(metrics, processingTime, method) {
    const metricsContainer = document.getElementById('metricsContainer');
    
    document.getElementById('psnrValue').textContent = metrics.psnr;
    document.getElementById('ssimValue').textContent = metrics.ssim;
    document.getElementById('sharpnessValue').textContent = metrics.sharpness;
    document.getElementById('timeValue').textContent = processingTime.toFixed(1);
    
    // Add method indicator
    const methodIndicator = document.createElement('div');
    methodIndicator.className = 'method-indicator';
    methodIndicator.style.cssText = `
        text-align: center;
        margin-top: 10px;
        padding: 8px;
        background: ${method === 'cpp' ? '#e6fffa' : '#fff5f5'};
        border: 1px solid ${method === 'cpp' ? '#4fd1c7' : '#f56565'};
        border-radius: 6px;
        font-size: 0.9em;
        color: ${method === 'cpp' ? '#234e52' : '#742a2a'};
    `;
    
    methodIndicator.innerHTML = `
        <i class="fas ${method === 'cpp' ? 'fa-microchip' : 'fa-flask'}"></i>
        ${method === 'cpp' ? 'C++ Backend' : 'Simulation Mode'}
    `;
    
    // Remove existing method indicator
    const existing = metricsContainer.querySelector('.method-indicator');
    if (existing) {
        existing.remove();
    }
    
    metricsContainer.appendChild(methodIndicator);
    metricsContainer.style.display = 'block';
}

function downloadEnhanced() {
    if (!enhancedImage) {
        showError('No enhanced image available for download');
        return;
    }
    
    // Create download link
    const link = document.createElement('a');
    link.href = enhancedImage;
    link.download = `enhanced_image_${Date.now()}.jpg`;
    
    // Trigger download
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    
    // Show success message
    showSuccess('Enhanced image downloaded successfully!');
}

function processAnother() {
    clearImage();
    document.getElementById('resultsSection').style.display = 'none';
    
    // Scroll back to top
    window.scrollTo({ top: 0, behavior: 'smooth' });
}

function showError(message) {
    document.getElementById('errorMessage').textContent = message;
    document.getElementById('errorModal').style.display = 'flex';
}

function closeErrorModal() {
    document.getElementById('errorModal').style.display = 'none';
}

function showSuccess(message) {
    // Create a temporary success notification
    const notification = document.createElement('div');
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        background: linear-gradient(135deg, #48bb78 0%, #38a169 100%);
        color: white;
        padding: 15px 20px;
        border-radius: 8px;
        box-shadow: 0 4px 15px rgba(72, 187, 120, 0.4);
        z-index: 1001;
        font-weight: 500;
        display: flex;
        align-items: center;
        gap: 10px;
    `;
    
    notification.innerHTML = `
        <i class="fas fa-check-circle"></i>
        ${message}
    `;
    
    document.body.appendChild(notification);
    
    // Remove after 3 seconds
    setTimeout(() => {
        notification.remove();
    }, 3000);
}

// Backend Integration Functions (for when C++ backend is ready)
async function callCppEnhancer(imageData, parameters) {
    // This function will be implemented when the C++ backend is ready
    // It will send the image and parameters to the C++ face enhancer
    
    const formData = new FormData();
    formData.append('image', imageData);
    formData.append('parameters', JSON.stringify(parameters));
    
    try {
        const response = await fetch('/api/enhance', {
            method: 'POST',
            body: formData
        });
        
        if (!response.ok) {
            throw new Error('Enhancement request failed');
        }
        
        const result = await response.json();
        return result;
        
    } catch (error) {
        throw new Error('Failed to communicate with enhancement service: ' + error.message);
    }
}

// Export functions for potential external use
window.FaceEnhancer = {
    enhanceImage,
    clearImage,
    downloadEnhanced,
    processAnother
};