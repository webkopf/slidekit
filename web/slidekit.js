var rootSlides = document.querySelectorAll("#slides > section");

var curSlideIdx;

var hash = location.hash.substr(1);
if(hash == "autoplay"){
	curSlideIdx = 0;	
	setInterval(nextSlideStep, 10000);
}
else{
	curSlideIdx = parseInt(location.hash.substr(1)) | 0;	
}

var curDataFragments = null;
var curDataFragmentIdx;




function nextSlideStep(){
		
	if(curDataFragmentIdx < curDataFragments.length){
		
		curDataFragments[curDataFragmentIdx].style.opacity = "1";
		curDataFragments[curDataFragmentIdx].style.visibility = "visible";
		curDataFragmentIdx++;
	}
	else{
//		rootSlides[curSlideIdx].style.display = "none";
		curSlideIdx = (curSlideIdx + 1) % rootSlides.length;		
		initDataFragments();
		location.hash = curSlideIdx;
		
		var slidesElem = document.getElementById("slides");
		
		slidesElem.style.webkitAnimation = "slideForward" + " 0.5s forwards";
		
		if(window.on_before_next_slide){
			on_before_next_slide();
		}
		
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
			
			if(window.on_after_next_slide){
				on_after_next_slide();
			}
			
		}, 600);
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
	curSlideIdx--;
	
	if(curSlideIdx < 0){
		curSlideIdx = rootSlides.length - 1;
	}
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

