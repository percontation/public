// Given a url string, extract and decode a query parameter.
function getParam(url, param) {
	var parameq = param+'=';
	var q = url.indexOf("?");
	if(q < 0) return undefined;
	var params = url.substring(q+1).split("&");
	for(var i = 0; i < params.length; i++) {
		var p = params[i];
		if(p.lastIndexOf(parameq, 0) == 0) {
			return decodeURIComponent(p.substring(parameq.length));
		}
	}
	return undefined;
}

// If link is a relay link, and the destination is in url query
// parameter param, replace the href with the destination.
function deRelay(link, param) {
	if(!link || !link.href) return;
	var x = getParam(link.href, param);
	if(x) link.href = x;
}

// If el is not a link, return the closest ancestor of el that is;
// or if el is a link, just return el.
function enclosingLink(el) {
	if(!el) return undefined;
	if(el.tagName == 'A') return el;
	return enclosingLink(el.parentElement);
}

// Assume our intended targets own name.* and name.*.*; it doesn't
// matter if this is too broad, since defusing does not mess with
// unsilly links.
var rdns = document.location.host.split(".").reverse();

if(rdns[1] == "google" || rdns[2] == "google") {
	var grelay = /google\.[^/]+\/url?/;
	function googledefuse(link) {
		if(link.onmousedown && link.onmousedown.toString().indexOf("return rwt(this,''") >= 0) {
			link.removeAttribute('onmousedown');
		}
		if(link.href && grelay.test(link.href)) {
			deRelay(link, "url");
		}
	}
	
	document.body.addEventListener('mousedown', function(event) {
		var t = enclosingLink(event.target);
		if(t) googledefuse(t);
	}, true);
	document.body.addEventListener('mousedown', function(event) {
		var t = enclosingLink(event.target);
		if(t) googledefuse(t);
	}, false);
}

if(rdns[1] == "facebook" || rdns[2] == "facebook") {
	var frelay = /facebook\.com\/l\.php?/;
	function facebookdefuse(link) {
		if(link.href && frelay.test(link.href)) {
			deRelay(link, "u");
			if(link.onmouseover && link.onmouseover.toString().indexOf("LinkshimAsyncLink.swap") >= 0) {
				link.removeAttribute('onmouseover');
			}
		}
		if(link.onclick && link.onclick.toString().indexOf("LinkshimAsyncLink.referrer_log") >= 0) {
			link.removeAttribute('onclick');
		}
		if(link.onmouseover && link.onmouseover.toString().indexOf("LinkshimAsyncLink.swap") >= 0) {
			link.onmouseover(); // Trigger the replacement of the link
			link.removeAttribute('onmouseover');
		}
	}
	
	document.body.addEventListener('mousedown', function(event) {
		var t = enclosingLink(event.target);
		if(t) facebookdefuse(t);
	}, true);
}
