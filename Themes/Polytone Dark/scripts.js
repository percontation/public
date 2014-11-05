"use strict";

var lch = (function() {
	// D65
	var X0 = 0.95047;
	var Z0 = 1.08883;
	
	// F2
	// var X0 = 0.99187;
	// var Z0 = 0.67395;
	
	function _lab1(t) {
	  return t > 6/29 ? t*t*t : 108/841 * (t - 4/29);
	}
	
	function _srgb(t) {
	  return t <= 0.0031308 ? 12.92*t : 1.055 * Math.pow(t, 1/2.4) - 0.055;
	}
	
	function _rgb0(t) {
	  t = Math.floor(256 * t);
	  return t <= 0 ? 0 : t >= 255 ? 255 : t;
	}
	
	return (function(L, C, H) {
		L /= 100;
		C /= 100;
		H *= Math.PI/180;

		var A = Math.cos(H) * C;
		var B = Math.sin(H) * C;

		var t = (L+0.16) / 1.16;
		var Y = _lab1(t);
		var X = X0 * _lab1(t + A/5);
		var Z = Z0 * _lab1(t - B/2);

		var R = _rgb0(_srgb( 3.2406*X + -1.5372*Y + -0.4986*Z));
		var G = _rgb0(_srgb(-0.9689*X +  1.8758*Y +  0.0415*Z));
		var B = _rgb0(_srgb( 0.0557*X + -0.2040*Y +  1.0570*Z));

		//return 'rgb('+R+','+G+','+B+')';
		return '#' + ((1<<24)|(R<<16)|(G<<8)|B).toString(16).substr(1).toUpperCase();
	});
})();

function thirtyColors() {
	// Loosely based off the optimal packing of 30 circles inside a circle
	var a = [];
	var r = 1/(2+Math.SQRT2+Math.sqrt(3));
	for(var i = 0; i < 4; i++) {
		a.push(lch(80, 70*Math.SQRT2*r, 360*i/4));
	}
	for(var i = 0; i < 10; i++) {
		var x = Math.cos(2*Math.PI*i/10) * (2+Math.SQRT2)*r;
		var y = Math.sin(2*Math.PI*i/10) * (Math.sqrt(3)+Math.SQRT2)*r;
		a.push(lch(70, 70*Math.sqrt(x*x+y*y), 360*i/10));
	}
	for(var i = 0; i < 16; i++) {
		a.push(lch(60, 70, 360*(i+0.5)/16));
	}
	return a;
}

var mappedSelectedUsers = new Array();

Textual.viewFinishedLoading = function() {
	setTimeout(function() { Textual.scrollToBottomOfView(); }, 0);

	var a = thirtyColors();
	var sheet = document.styleSheets[0];
	for(var i = 0; i < 30; i++) {
		sheet.insertRule(
			"body .sender[mtype*=normal][colornumber='"+i+"'], " +
			"body .inline_nickname[colornumber='"+i+"'] { " +
			"	color: "+a[i]+"; " +
			"}", sheet.cssRules.length);
	}
}

Textual.viewFinishedReload = function() {
	Textual.viewFinishedLoading();
}

Textual.newMessagePostedToView = function(line) {
	var element = document.getElementById("line-" + line);
	updateNicknameAssociatedWithNewMessage(element);
}

Textual.nicknameSingleClicked = function(e) {
	userNicknameSingleClickEvent(e);
}

function updateNicknameAssociatedWithNewMessage(e) {
	/* We only want to target plain text messages. */
	var elementType = e.getAttribute("ltype");

	if (elementType == "privmsg" || elementType == "action") {
		/* Get the nickname information. */
		var senderSelector = e.querySelector(".sender");

		if (senderSelector) {
			/* Is this a mapped user? */
			var nickname = senderSelector.getAttribute("nickname");

			/* If mapped, toggle status on for new message. */
			if (mappedSelectedUsers.indexOf(nickname) > -1) {
				toggleSelectionStatusForNicknameInsideElement(senderSelector);
			}
		}
	}
}

function toggleSelectionStatusForNicknameInsideElement(e) {
	var parentSelector = e.parentNode.parentNode.parentNode.parentNode;
	parentSelector.classList.toggle("selectedUser");
}

function userNicknameSingleClickEvent(e) {
	/* This is called when the .sender is clicked. */
	var nickname = e.getAttribute("nickname");
	
	/* Toggle mapped status for nickname. */
	var mappedIndex = mappedSelectedUsers.indexOf(nickname);
	
	if (mappedIndex == -1) {
		mappedSelectedUsers.push(nickname);
	} else {
		mappedSelectedUsers.splice(mappedIndex, 1);
	}
	
	/* Gather basic information. */
	var documentBody = document.getElementById("body_home");
	
	var allLines = documentBody.querySelectorAll('div[ltype="privmsg"], div[ltype="action"]');
	
	/* Update all elements of the DOM matching conditions. */
	for (var i = 0, len = allLines.length; i < len; i++) {
		var sender = allLines[i].querySelectorAll(".sender");
	
		if (sender.length > 0) {
			if (sender[0].getAttribute("nickname") === nickname) {
				toggleSelectionStatusForNicknameInsideElement(sender[0]);
			}
		}
	}
}
