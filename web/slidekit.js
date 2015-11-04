var defaultStepDuration = 5;
var rootSlides = document.querySelectorAll("#slides > section");
var curSlideIdx;
var autoplay = false;
var nextSlideStepTimeout;


var hash = location.hash.substr(1);
if(hash == "autoplay"){
	curSlideIdx = 0;
	autoplay = true;
	var durationAttr = rootSlides[curSlideIdx].getAttribute("data-duration")
	var duration = parseInt(durationAttr) | defaultStepDuration;	
	nextSlideStepTimeout = setTimeout(nextSlideStep, duration * 1000);
	
}
else{
	curSlideIdx = parseInt(location.hash.substr(1)) | 0;	
}
rootSlides[curSlideIdx].classList.add("active");

var curDataFragments = null;
var curDataFragmentIdx;




function nextSlideStep(){
	clearTimeout(nextSlideStepTimeout);
	
	var stepDuration;
		
	if(curDataFragmentIdx < curDataFragments.length){
		var fragment = curDataFragments[curDataFragmentIdx];
		fragment.style.opacity = "1";
		fragment.style.visibility = "visible";
		
		stepDuration = fragment.getAttribute("data-duration");
		
		curDataFragmentIdx++;
		
		
	}
	else{
//		rootSlides[curSlideIdx].style.display = "none";
		curSlideIdx = (curSlideIdx + 1) % rootSlides.length;		
		initDataFragments();
		location.hash = curSlideIdx;
		stepDuration = rootSlides[curSlideIdx].getAttribute("data-duration");
		
		var slidesElem = document.getElementById("slides");				
		slidesElem.style.webkitAnimation = "slideForward" + " 0.5s forwards";
		
		//notify slidekit-binary, if it has injected according method
		if(window.on_before_next_slide){
			on_before_next_slide();
		}
		
		// refreshes the "active"-state after the animation, so the next slide becomes
		// rendered for the "slide-in-animation"
		setTimeout(function(){
			rootSlides[curSlideIdx].classList.add("active");
			
			var previousSlideIdx;
			if(curSlideIdx == 0){
				previousSlideIdx = rootSlides.length - 1; 
			}
			else{
				previousSlideIdx = curSlideIdx - 1;
			}
						
			rootSlides[previousSlideIdx].classList.remove("active");
			slidesElem.style.webkitAnimation = "";
			
			//notify slidekit-binary, if it has injected according method
			if(window.on_after_next_slide){
				on_after_next_slide();
			}
			
		}, 600);
	}
	
	if(autoplay){
		if(stepDuration == null){
			stepDuration = defaultStepDuration;
		}
		nextSlideStepTimeout = setTimeout(nextSlideStep, stepDuration * 1000);
	}
	
	
}

function initDataFragments(){
	curDataFragmentIdx = 0;
	curDataFragments = rootSlides[curSlideIdx].querySelectorAll("*[data-fragment]");
	for(var i = 0; i < curDataFragments.length; i++){
		curDataFragments[i].style.opacity = "0"; //damit beim Sliden kein Ausfaden sichtbar wird

		curDataFragments[i].style.visibility = "hidden"; //damit beim Sliden kein Ausfaden sichtbar wird

	}
};
initDataFragments();

function previousSlideStep(){
	clearTimeout(nextSlideStepTimeout);
	
	rootSlides[curSlideIdx].classList.remove("active");
	curSlideIdx--;
	
	if(curSlideIdx < 0){
		curSlideIdx = rootSlides.length - 1;
	}
	rootSlides[curSlideIdx].classList.add("active");
	location.hash = curSlideIdx;
	var term = "s" + curSlideIdx;
	document.body.className = term;
}

window.onkeyup = function(evt){
	
	console.log(evt.keyCode);
	switch(evt.keyCode){
		case 39: //ARROW_RIGHT
			nextSlideStep();
			break;
		case 37: //ARROW_LEFT
			previousSlideStep();
			break;
	}	
}

