var data;
var intext;

data = ["1	she| like , eat , | shit"];
intext = "My sister's name is Linda, she likes eat shit.";

function submitCorpus() {
    intext = document.getElementById('inText').value;
    document.getElementById("outputImg").style.display = "none";
    let selection = document.getElementById('selection').value;
    let xhr = new XMLHttpRequest();
    xhr.open("POST", "http://dmserv4.cs.illinois.edu:1111/remine", true);
    xhr.setRequestHeader('Content-Type', 'application/json')
    var sendText = JSON.parse('{ "text":"" }');
    sendText.text = intext;
    xhr.send(JSON.stringify(sendText));
    xhr.onload = function (e) {
        if (xhr.readyState === 4) {
            if (xhr.status === 200) {
              data = JSON.parse(xhr.responseText).tuple;
              let list = document.createElement('ul');
              for(let i = 0; i < data.length; i++) {
                  let item = document.createElement('li');
                  item.appendChild(document.createTextNode(data[i]));
                  list.appendChild(item);
              }
              if (document.getElementById("outText").childNodes.length > 0) {
                  document.getElementById("outText").replaceChild(list, document.getElementById("outText").childNodes[0]);
              } else {
                  document.getElementById("outText").appendChild(list);
              }
              xhr.abort();
            } else {
              console.error(xhr.statusText);
              xhr.abort();
            }
        }
    };

    if (button_state === 1) {
        pass_reslut = "";
        for (let i = 0; i < data.length; i++) {
            if (i === data.length - 1) {
                pass_reslut = pass_reslut + data[i];
            } else {
                pass_reslut = pass_reslut + data[i] + "\n";
            }
        }
        $.ajax({
            type: "POST",
            url: "/cof",
            data: { origin: intext, result: pass_reslut }
        }).done(function(e) {
            e = e.tuple;
            let list = document.createElement('ul');
            for(let i = 0; i < e.length; i++) {
                let item = document.createElement('li');
                item.appendChild(document.createTextNode(e[i]));
                list.appendChild(item);
            }
            if (document.getElementById("outText").childNodes.length > 0) {
                document.getElementById("outText").replaceChild(list, document.getElementById("outText").childNodes[0]);
            } else {
                document.getElementById("outText").appendChild(list);
            }
        });
    }
}




// button
var button_state = 0;
window.onload = function () {
    change_state();
    document.getElementById("b1").addEventListener("click", function() {
        button_state = 0;
        change_state();
    });
    document.getElementById("b2").addEventListener("click", function() {
        button_state = 1;
        change_state();
    });
    document.getElementById("b3").addEventListener("click", function() {
        button_state = 2;
        change_state();
    });

};

function change_state() {
    let bs = document.getElementsByClassName("buttons");
    for (let i = 0; i < bs.length; i++) {
        if (button_state === i) {
            bs[i].style.backgroundColor = "#F2F2F2";
            document.getElementById("intro_title").innerHTML = bs[i].innerHTML;
        } else {
            bs[i].style.backgroundColor = "white";
        }
    }
}
