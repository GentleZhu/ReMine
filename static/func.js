var data;
var intext;

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
              if (button_state == 0) {
                  let list = document.createElement('ul');
                  let index = "1";
                  for(let i = 0; i < data.length; i++) {
                      let temp = index;
                      if (data[i][0] !== index) {
                          let button = document.createElement('a');
                          button.innerHTML = 'click me';
                          button.style.cssText = "margin-left: 40vw; color: #0084FF";
                          button.addEventListener('click', function(){
                              call_vi(temp);
                          });
                          list.appendChild(button);
                          index = data[i][0];
                      }
                      let item = document.createElement('li');
                      item.appendChild(document.createTextNode(data[i]));
                      list.appendChild(item);
                  }
                  let button = document.createElement('a');
                  button.innerHTML = 'click me';
                  button.style.cssText = "margin-left: 40vw; color: #0084FF";
                  button.addEventListener('click', function(){
                      call_vi(index);
                  });
                  list.appendChild(button);
                  if (document.getElementById("outText").childNodes.length > 0) {
                      document.getElementById("outText").replaceChild(list, document.getElementById("outText").childNodes[0]);
                  } else {
                      document.getElementById("outText").appendChild(list);
                  }
              }
              xhr.abort();
            } else {
              console.error(xhr.statusText);
              xhr.abort();
            }
        }
    };

    if (button_state === 1) {
        let pass_reslut = "";
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
            let index = "1";
            for(let i = 0; i < e.length; i++) {
                let temp = index;
                if (data[i][0] !== index) {
                    let button = document.createElement('a');
                    button.innerHTML = 'click me';
                    button.style.cssText = "margin-left: 40vw; color: #0084FF";
                    button.addEventListener('click', function(){
                        call_vi(temp);
                    });
                    list.appendChild(button);
                    index = data[i][0];
                }
                let item = document.createElement('li');
                item.appendChild(document.createTextNode(e[i]));
                list.appendChild(item);
            }
            let button = document.createElement('a');
            button.innerHTML = 'click me';
            button.style.cssText = "margin-left: 40vw; color: #0084FF";
            button.addEventListener('click', function(){
                call_vi(index);
            });
            list.appendChild(button);
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

// data vi
function call_vi(index) {
    document.getElementById("input_").style.display = "none";
    let clear = document.createElement('a');
    clear.innerHTML = 'Go Back';
    clear.style.cssText = "position: absolute; top: 2vmin; left: 5vmin; font-family: 'Open Sans', sans-serif; font-size: 3vmin;";
    document.getElementById("input").appendChild(clear);
    clear.addEventListener('click', function(){
        document.getElementById("input_").style.display = "block";
        document.getElementById("input").removeChild(clear);
        d3.select("svg").remove();
    });
    let pass_reslut = "";
    for (let i = 0; i < data.length; i++) {
        if (data[i][0] === index) {
            pass_reslut = pass_reslut + data[i] + "\n";
        }
    }
    $.ajax({
        type: "POST",
        url: "/vi",
        data: { result: pass_reslut }
    }).done(function(e) {
        vi_data = e.tuple;
        let nodes = [];
        let nodes_name = [];
        let edges = [];
        let label = [];
        let d = {};
        let count = 0;
        for (let i = 0; i < vi_data.length; i++) {
            let tup = vi_data[i];
            if (!nodes.includes(tup[1])) {
                nodes.push(tup[1]);
                d[tup[1]] = count;
                count += 1;
            }
            if (!nodes.includes(tup[3])) {
                nodes.push(tup[3]);
                d[tup[3]] = count;
                count += 1;
            }
            let relation = "";
            for (let j = 0; j < tup[2].length; j++) {
                relation = relation + tup[2][j] + ",";
            }
            label.push(relation);
            edges.push({source: d[tup[1]], target: d[tup[3]]})
        }
        for (let k = 0; k < nodes.length; k++) {
            nodes_name.push({name: nodes[k]});
        }
        let dataset = {
            nodes: nodes_name,
            edges: edges,
            label : label
        };
        vi(dataset);
    });
};



function vi(dataset) {
    var w = window.innerWidth * 0.5;
    var h = window.innerHeight * 0.9;
    var linkDistance=300;

    var colors = d3.scale.category10();

    d3.select("svg").remove();
    var svg = d3.select('[id="input"]').append("svg").attr({"width":w,"height":h});

    var force = d3.layout.force()
        .nodes(dataset.nodes)
        .links(dataset.edges)
        .size([w,h])
        .linkDistance([linkDistance])
        .charge([-500])
        .theta(0.1)
        .gravity(0.05)
        .start();



    var edges = svg.selectAll("line")
      .data(dataset.edges)
      .enter()
      .append("line")
      .attr("id",function(d,i) {return 'edge'+i})
      .attr('marker-end','url(#arrowhead)')
      .style("stroke","#ccc")
      .style("pointer-events", "none");

    var nodes = svg.selectAll("circle")
      .data(dataset.nodes)
      .enter()
      .append("circle")
      .attr({"r":20})
      .style("fill",function(d,i){return colors(i);})
      .call(force.drag)


    var nodelabels = svg.selectAll(".nodelabel")
       .data(dataset.nodes)
       .enter()
       .append("text")
       .attr({"x":function(d){return d.x;},
              "y":function(d){return d.y;},
              "class":"nodelabel",
              "stroke":"black",
              'font-size':25})
       .text(function(d){return d.name;});

    var edgepaths = svg.selectAll(".edgepath")
        .data(dataset.edges)
        .enter()
        .append('path')
        .attr({'d': function(d) {return 'M '+d.source.x+' '+d.source.y+' L '+ d.target.x +' '+d.target.y},
               'class':'edgepath',
               'fill-opacity':0,
               'stroke-opacity':0,
               'fill':'blue',
               'stroke':'red',
               'id':function(d,i) {return 'edgepath'+i}})
        .style("pointer-events", "none");

    var edgelabels = svg.selectAll(".edgelabel")
        .data(dataset.edges)
        .enter()
        .append('text')
        .style("pointer-events", "none")
        .attr({'class':'edgelabel',
               'id':function(d,i){return 'edgelabel'+i},
               'dx':120,
               'dy':0,
               'font-size':18,
               'fill':'#aaa'});

    edgelabels.append('textPath')
        .attr('xlink:href',function(d,i) {return '#edgepath'+i})
        .style("pointer-events", "none")
        .text(function(d,i){return dataset.label[i]});


    svg.append('defs').append('marker')
        .attr({'id':'arrowhead',
               'viewBox':'-0 -5 10 10',
               'refX':25,
               'refY':0,
               //'markerUnits':'strokeWidth',
               'orient':'auto',
               'markerWidth':10,
               'markerHeight':10,
               'xoverflow':'visible'})
        .append('svg:path')
            .attr('d', 'M 0,-5 L 10 ,0 L 0,5')
            .attr('fill', '#ccc')
            .attr('stroke','#ccc');


    force.on("tick", function(){

        edges.attr({"x1": function(d){return d.source.x;},
                    "y1": function(d){return d.source.y;},
                    "x2": function(d){return d.target.x;},
                    "y2": function(d){return d.target.y;}
        });

        nodes.attr({"cx":function(d){return d.x;},
                    "cy":function(d){return d.y;}
        });

        nodelabels.attr("x", function(d) { return d.x; })
                  .attr("y", function(d) { return d.y; });

        edgepaths.attr('d', function(d) { var path='M '+d.source.x+' '+d.source.y+' L '+ d.target.x +' '+d.target.y;
                                           //console.log(d)
                                           return path});

        edgelabels.attr('transform',function(d,i){
            if (d.target.x<d.source.x){
                bbox = this.getBBox();
                rx = bbox.x+bbox.width/2;
                ry = bbox.y+bbox.height/2;
                return 'rotate(180 '+rx+' '+ry+')';
                }
            else {
                return 'rotate(0)';
                }
        });
    });
}
