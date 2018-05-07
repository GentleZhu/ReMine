// var data=["1	three man | have die , | medical error",
// "1	three man | have die , in , | the previous month",
// "10	Nev. | will , be ask , they , | question",
// "10	Nev. | sponsor , | municipal employee",
// "10	Nev. | sponsor , | County",
// "11	here on Thursday | between , | the United States",
// "11	Europe -- | and , | Iran and Syria",
// "11	here on Thursday | of , | proxy war",
// "11	here on Thursday | | a snapshot"];
var data_re;
var data_cor;
var origin;
var remine_list;
var remine_cor_list;
// Process
function submitCorpus() {
    let timer = setTimeout(function(){
        alert("Your document maybe too long. Please refresh and try again! 🙂🙂🙂🙂🙂🙂🙂");
    },150000);
    clear_vi();
    var intext = document.getElementById('inText').value;
    if (intext.length === 0) {
        alert("You need to type something here to submit 凸😤凸")
        return;
    }
    document.getElementById("outputImg").style.display = "none";
    let selection = document.getElementById('selection').value;
    let xhr = new XMLHttpRequest();
    xhr.open("POST", "http://dmserv4.cs.illinois.edu:1111/remine", true);
    xhr.setRequestHeader('Content-Type', 'application/json')
    let sendText = JSON.parse('{ "text":"", "model":"" }');
    sendText.text = intext;
    sendText.model = selection;
    let list = document.getElementsByTagName("ul"), index;
    for (index = list.length - 1; index >= 0; index--) {
        list[index].parentNode.removeChild(list[index]);
    }
    document.getElementById("loader").style.display = "block";
    xhr.send(JSON.stringify(sendText));
    xhr.onload = function (e) {
        if (xhr.readyState === 4) {
            if (xhr.status === 200) {
                data = JSON.parse(xhr.responseText).tuple;
                origin = JSON.parse(xhr.responseText).lemma;
                if (button_state == 0) {
                    clearTimeout(timer);
                    data_re = data;
                    document.getElementById("loader").style.display = "none";
                    let list = document.createElement('ul');
                    let index = get_index(data_re[0]);
                    for(let i = 0; i < data_re.length; i++) {
                        let temp = index;
                        let tup_index = get_index(data_re[i]);
                        if (tup_index !== index) {
                            let button = document.createElement('a');
                            button.innerHTML = 'click me';
                            button.style.cssText = "margin-left: 40vw; color: #0084FF";
                            button.addEventListener('click', function(){
                                call_vi(temp);
                            });
                            list.appendChild(button);
                            index = tup_index;
                        }
                        let item = document.createElement('li');
                        item.appendChild(document.createTextNode(data_re[i]));
                        list.appendChild(item);
                    }
                    if (list.childNodes.length > 0) {
                        let button = document.createElement('a');
                        button.innerHTML = 'click me';
                        button.style.cssText = "margin-left: 40vw; color: #0084FF";
                        button.addEventListener('click', function(){
                            call_vi(index);
                        });
                        list.appendChild(button);
                    }
                    remine_list = list;
                    if (document.getElementById("outText").childNodes.length > 4) {
                        document.getElementById("outText").replaceChild(list, document.getElementById("outText").childNodes[4]);
                    } else {
                        document.getElementById("outText").appendChild(list);
                    }
                }
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
                        data: { origin: origin, result: pass_reslut }
                    }).done(function(e) {
                        clearTimeout(timer);
                        document.getElementById("loader").style.display = "none";
                        data_cor = e.tuple;
                        let list = document.createElement('ul');
                        let index = get_index(data_cor[0]);
                        for(let i = 0; i < data_cor.length; i++) {
                            let tup_index = get_index(data_cor[i]);
                            let temp = index;
                            if (tup_index !== index) {
                                let button = document.createElement('a');
                                button.innerHTML = 'click me';
                                button.style.cssText = "margin-left: 40vw; color: #0084FF";
                                button.addEventListener('click', function(){
                                    call_vi(temp);
                                });
                                list.appendChild(button);
                                index = tup_index;
                            }
                            let item = document.createElement('li');
                            item.appendChild(document.createTextNode(data_cor[i]));
                            list.appendChild(item);
                        }
                        if (list.childNodes.length > 0) {
                            let button = document.createElement('a');
                            button.innerHTML = 'click me';
                            button.style.cssText = "margin-left: 40vw; color: #0084FF";
                            button.addEventListener('click', function(){
                                call_vi(index);
                            });
                            list.appendChild(button);
                        }
                        remine_cor_list = list;
                        if (document.getElementById("outText").childNodes.length > 4) {
                            document.getElementById("outText").replaceChild(list, document.getElementById("outText").childNodes[4]);
                        } else {
                            document.getElementById("outText").appendChild(list);
                        }
                    });
                }
                xhr.abort();
            } else {
                console.error(xhr.statusText);
                xhr.abort();
            }
        }
    };


}

function get_index(e) {
    let res = "";
    let start = 0;
    while (e[start] !== "\t") {
        res = res + e[start];
        start += 1;
    }
    return res;
}

function reset() {
    button_state = 0;
    change_state();
    remine_list = null;
    remine_cor_list = null;
    let list = document.getElementsByTagName("ul"), index;
    for (index = list.length - 1; index >= 0; index--) {
        list[index].parentNode.removeChild(list[index]);
    }
    document.getElementById("outputImg").style.display = "block";
    clear_vi();
    document.getElementById('inText').value = "";
}

function save_list(e) {
    document.getElementById("outputImg").style.display = "block";
    let list = document.getElementsByTagName("ul"), index;
    for (index = list.length - 1; index >= 0; index--) {
        list[index].parentNode.removeChild(list[index]);
    }
    if (e) {
        document.getElementById("outputImg").style.display = "none";
        if (document.getElementById("outText").childNodes.length > 4) {
            document.getElementById("outText").replaceChild(e, document.getElementById("outText").childNodes[4]);
        } else {
            document.getElementById("outText").appendChild(e);
        }
    }
    clear_vi();
}

function clear_vi() {
    document.getElementById("outText").style.height = "90vh";
    d3.select("svg").remove();
    console.log(document.getElementById("output").childNodes);
    while (document.getElementById("output").childNodes.length > 3) {
        let node = document.getElementById("output");
        document.getElementById("output").removeChild(node.lastChild);
    }
}


// Feature button
var button_state = 0;
window.onload = function () {
    change_state();
    document.getElementById("b1").addEventListener("click", function() {
        button_state = 0;
        change_state();
        save_list(remine_list);
    });
    document.getElementById("b2").addEventListener("click", function() {
        button_state = 1;
        change_state();
        save_list(remine_cor_list);
    });
    document.getElementById("b3").addEventListener("click", function() {
        button_state = 2;
        change_state();
    });
    document.getElementById("home").addEventListener('click', function(){
        reset();
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
    document.getElementById("outText").style.height = "30vh";
    let clear = document.createElement('a');
    clear.innerHTML = 'Clear';
    clear.style.cssText = "position: absolute; z-index: 100; top: 32vh; left: 5vmin; font-family: 'Open Sans', sans-serif; font-size: 3vmin; color: #0084FF;";
    document.getElementById("output").appendChild(clear);
    clear.addEventListener('click', function(){
        clear_vi();
    });
    let pass_reslut = "";
    let data_vi = data_re;
    if (button_state == 1) {
        data_vi = data_cor;
    }
    for (let i = 0; i < data_vi.length; i++) {
        let tup_index = get_index(data_vi[i]);
        if (tup_index === index) {
            pass_reslut = pass_reslut + data_vi[i] + "\n";
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
    var h = window.innerHeight * 0.6;
    var rate = window.innerWidth;
    var linkDistance= rate * 0.14;

    var colors = d3.scale.category10();

    d3.select("svg").remove();
    var svg = d3.select('[id="output"]').append("svg").attr({"width":w,"height":h, "style": "position:absolute; border-top:0.1px solid #D9D9D9; top:30vh"});

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
      .attr({"r":rate * 0.01})
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
              'font-size':rate * 0.015})
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
               'dx':function(d,i){return (linkDistance - rate * 0.005 * dataset.label[i].length) / 2},
               'dy':0,
               "stroke": "grey",
               'font-size':rate * 0.01,
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
