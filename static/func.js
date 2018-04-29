function submitCorpus() {
    let intext = document.getElementById('inText').value;
    let selection = document.getElementById('selection').value;
    console.log(intext);
    console.log(selection);
    let xhr = new XMLHttpRequest();
    xhr.open("POST", "http://dmserv4.cs.illinois.edu:1111/remine", true);
    xhr.setRequestHeader('Content-Type', 'application/json')
    var sendText = JSON.parse('{ "text":"" }');
    sendText.text = intext;
    console.log(sendText);
    xhr.send(intext);
    xhr.onload = function (e) {
        if (xhr.readyState === 4) {
            if (xhr.status === 200) {
              let data = JSON.parse(xhr.responseText).tuple;
              var list = document.createElement('ul');
              for(var i = 0; i < data.length; i++) {
                  var item = document.createElement('li');
                  item.appendChild(document.createTextNode(data[i]));
                  list.appendChild(item);
              }
              document.getElementById("outText").appendChild(list);
              xhr.abort();
            } else {
              console.error(xhr.statusText);
              xhr.abort();
            }
        }
    };
}
