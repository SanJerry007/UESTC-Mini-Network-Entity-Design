var textarea = document.querySelector("textarea");
var send = document.querySelector("button");
var ul = document.querySelector("ul");
var rPos = 1;
// var sPos = 1;
var display = document.querySelector(".display");
// var userHead = document.querySelector("span")

// userHead.onclick = function(){
// console.log("head");
// }
function load(name) {
  let xhr = new XMLHttpRequest(),
    okStatus = document.location.protocol === "file:" ? 0 : 200;
  xhr.open("GET", name, false);
  xhr.overrideMimeType("text/html;charset=utf-8"); //默认为utf-8
  xhr.send(null);
  var text = xhr.status === okStatus ? xhr.responseText : null;
  if (text != null) {
    var li = document.createElement("li");
    li.innerHTML = "<span></span>" + text;
    if (name[name.length - 5] == "z") {
      li.className = "right";
    }
    ul.appendChild(li);
    return true;
  } else {
    return false;
  }
}
// load("../../d1.txt");
setInterval(() => {
  if (load("../infs/" + rPos + "-d.txt")) {
    rPos++;
    display.scrollTop = display.scrollHeight;
  } else if (load("../infs/" + rPos + "-z.txt")) {
    rPos++;
    display.scrollTop = display.scrollHeight;
  }
}, 10);

document
  .querySelector("textarea")
  .addEventListener("keydown", function (event) {
    var e = event || window.event || arguments.callee.caller.arguments[0];
    if (e && e.keyCode == 27) {
      // 按 Esc
      //要做的事情
      console.log("按 esc");
    }
    if (e && e.keyCode == 113) {
      // 按 F2
      //要做的事情
      console.log("按 f2");
    }
    if (e && e.keyCode == 13) {
      // enter 键
      //要做的事情
      // console.log("按 Enter");
    }
    if (e.keyCode == 86 && e.ctrlKey) {
      console.log("你按下了ctrl+V");
    }
    if (e.keyCode == 13 && e.ctrlKey) {
      var content = textarea.value;
      //var li = document.createElement("li");
      // li.innerHTML = "<span></span>" + content;
      // li.className = "right";
      // ul.appendChild(li);
      var ul = document.querySelector("ul");
      var lis = ul.getElementsByTagName("li");
      textarea.value = "";
      download(lis.length + 1 + "-z.txt", content);
    }
  });

//写入
function download(filename, text) {
  var element = document.createElement("a");
  element.setAttribute(
    "href",
    "data:text/plain;charset=utf-8," + encodeURIComponent(text)
  );
  element.setAttribute("download", filename);

  element.style.display = "none";
  document.body.appendChild(element);

  element.click();

  document.body.removeChild(element);
}
